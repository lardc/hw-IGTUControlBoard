﻿// Header
#include "Controller.h"
//
// Includes
#include "Board.h"
#include "DeviceProfile.h"
#include "Interrupts.h"
#include "LowLevel.h"
#include "SysConfig.h"
#include "DebugActions.h"
#include "Diagnostic.h"
#include "BCCIxParams.h"
#include "Measurement.h"
#include "math.h"
#include "Delay.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
static Boolean CycleActive = false;
//
volatile Int64U CONTROL_TimeCounter = 0;
volatile Int16U CONTROL_TimerMaxCounter = 0;
volatile Int64U CONTROL_C_TimeCounter = 0;
volatile Int64U CONTROL_C_Start_Counter = 0;
volatile Int64U CONTROL_C_Stop_Counter = 0;
//
volatile Int16U CONTROL_V_Values_Counter = 0;
volatile Int16U CONTROL_C_Values_Counter = 0;
volatile Int16U CONTROL_V_VValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_VSenValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_RegOutValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_RegErrValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_VDACRawValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_CSenValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_C_CSenValues[C_VALUES_x_SIZE];
//
volatile RegulatorParamsStruct RegulatorParams;
static FUNC_AsyncDelegate LowPriorityHandle = NULL;
/// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
void CONTROL_LogicProcess();
void CONTROL_StopProcess();
void CONTROL_PostPulseSlowSequence();
void CONTROL_ResetOutputRegisters();
bool CONTROL_RegulatorCycle(volatile RegulatorParamsStruct* Regulator);
void CONTROL_StartPrepare();
void CONTROL_CashVariables();
bool CONTROL_BatteryVoltageCheck();

// Functions
//
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[EP_COUNT] = {EP_V_V_FORM, EP_V_V_MEAS_FORM, EP_REGULATOR_OUTPUT, EP_REGULATOR_ERR,
			EP_V_DAC_RAW_DATA,
			EP_V_C_MEAS_FORM, EP_C_C_FORM};

	Int16U EPSized[EP_COUNT] = {V_VALUES_x_SIZE, V_VALUES_x_SIZE, V_VALUES_x_SIZE, V_VALUES_x_SIZE, V_VALUES_x_SIZE,
			V_VALUES_x_SIZE, C_VALUES_x_SIZE};

	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter,
			(pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter,
			(pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_C_Values_Counter};

	pInt16U EPDatas[EP_COUNT] = {(pInt16U)CONTROL_V_VValues, (pInt16U)CONTROL_V_VSenValues,
			(pInt16U)CONTROL_V_RegOutValues, (pInt16U)CONTROL_V_RegErrValues, (pInt16U)CONTROL_V_VDACRawValues,
			(pInt16U)CONTROL_V_CSenValues, (pInt16U)CONTROL_C_CSenValues};

	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, CAN_NID);

	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Сброс значений
	DEVPROFILE_ResetControlSection();
	CONTROL_ResetToDefaultState();

	CU_LoadConvertParams();
}
//------------------------------------------

void CONTROL_ResetOutputRegisters()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;

	DataTable[REG_VGS] = 0;
	DataTable[REG_QG] = 0;
	DataTable[REG_QG_T] = 0;
	DataTable[REG_QG_C] = 0;
	DataTable[REG_IGES] = 0;

	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();
}
//------------------------------------------

void CONTROL_C_ResetArray()
{
	for(Int16U i = 0; i < C_VALUES_x_SIZE; i++)
	{
		CONTROL_C_CSenValues[i] = 0;
	}
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	LL_V_ShortOut(true);
	CONTROL_SetDeviceState(DS_None, SS_None);
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_LogicProcess();

	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateWatchDog();

	if(LowPriorityHandle)
	{
		LowPriorityHandle();
		LowPriorityHandle = NULL;
	}
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
			{
				CONTROL_SetDeviceState(DS_None, SS_SelfTest);
			}
			else if(CONTROL_State != DS_Ready)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if((CONTROL_State == DS_Ready) || (CONTROL_State == DS_InProcess))
				CONTROL_ResetToDefaultState();
			else if(CONTROL_State != DS_None)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_VGS_START:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_SetDeviceState(DS_InProcess, SS_VgsPulse);
				CONTROL_VGS_StartProcess();
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_QG_START:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_SetDeviceState(DS_InProcess, SS_QgPulse);
				CONTROL_QG_StartProcess();
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_IGES_START:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_SetDeviceState(DS_InProcess, SS_VgsPulse);
				CONTROL_IGES_StartProcess();
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_STOP_PROCESS:
			if(CONTROL_State == DS_InProcess)
			{
				CONTROL_V_StopProcess();
				CONTROL_C_StopProcess();
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			break;

		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
			{
				CONTROL_SetDeviceState(DS_None, SS_None);
				DataTable[REG_FAULT_REASON] = DF_NONE;
			}
			break;

		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		default:
			return DIAG_HandleDiagnosticAction(ActionID, pUserError);
			
	}
	return true;
}
//-----------------------------------------------

void CONTROL_LogicProcess()
{
	switch(CONTROL_SubState)
	{
		case SS_VgsWaitAfterPulse:
			CONTROL_VGS_SetResults(&RegulatorParams);
			break;

		case SS_IgesWaitAfterPulse:
			CONTROL_IGES_SetResults(&RegulatorParams);
			break;

		case SS_QgWaitAfterPulse:
			CONTROL_QG_SetResults();
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_V_HighPriorityProcess()
{
	switch(CONTROL_SubState)
	{
		case SS_VgsPulse:
			if(MEASURE_VGS_Params(&RegulatorParams, true))
				REGULATOR_VGS_FormUpdate(&RegulatorParams);

			if(CONTROL_RegulatorCycle(&RegulatorParams))
			{
				CONTROL_V_StopProcess();
				CONTROL_SetDeviceState(DS_InProcess, SS_VgsWaitAfterPulse);
			}
			break;

		case SS_IgesPulse:
			MEASURE_IGES_Params(&RegulatorParams, true);
			if(CONTROL_RegulatorCycle(&RegulatorParams))
			{
				CONTROL_V_StopProcess();
				CONTROL_SetDeviceState(DS_InProcess, SS_IgesWaitAfterPulse);
			}
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_C_HighPriorityProcess()
{
	if(CONTROL_SubState == SS_QgPulse)
	{
		TIM_Stop(TIM4);
		LL_C_CStart(true);
		LL_C_CSetDAC(0);
		TIM_Stop(TIM6);
		TIM_Reset(TIM6);
		ADC_SamplingStop(ADC1);
		DMA_TransferCompleteReset(DMA1, DMA_ISR_TCIF1);
		TIM_StatusClear(TIM4);
		CONTROL_SetDeviceState(DS_InProcess, SS_QgWaitAfterPulse);
	}
}
//-----------------------------------------------

bool CONTROL_RegulatorCycle(volatile RegulatorParamsStruct* Regulator)
{
	return REGULATOR_Process(Regulator);
}
//-----------------------------------------------

void CONTROL_VGS_StartProcess()
{
	CONTROL_ResetOutputRegisters();
	LL_V_ShortPAU(true);
	LL_V_ShortOut(false);
	MEASURE_C_CDMABufferClear();
	REGULATOR_VGS_FormConfig(&RegulatorParams);
	CONTROL_V_StartProcess();
}
//-----------------------------------------------

void CONTROL_IGES_StartProcess()
{
	CONTROL_ResetOutputRegisters();
	LL_V_ShortPAU(false);
	LL_V_ShortOut(false);
	REGULATOR_IGES_FormConfig(&RegulatorParams);
	CONTROL_V_StartProcess();
}
//-----------------------------------------------

void CONTROL_V_StartProcess()
{
	MEASURE_C_CDMABufferClear();
	REGULATOR_CashVariables(&RegulatorParams);
	CONTROL_ResetOutputRegisters();

	TIM_Reset(TIM15);
	TIM_Start(TIM15);
}
//-----------------------------------------------

void CONTROL_QG_StartProcess()
{
	CONTROL_ResetOutputRegisters();
	CONTROL_C_ResetArray();
	CONTROL_C_Values_Counter = 0;
	CONTROL_TimerMaxCounter = (Int16U)((float)DataTable[REG_QG_T_CURRENT] / (float)TIMER4_uS);
	LL_ExDACVCutoff((float)DataTable[REG_QG_V_CUTOFF]);
	LL_ExDACVNegative((float)DataTable[REG_QG_V_NEGATIVE]);
	LL_C_CSetDAC(CU_C_CToDAC((float)DataTable[REG_QG_C_SET]));
	DELAY_US(5);
	CONTROL_C_TimeCounter = 0;
	TIM_Reset(TIM4);
	DMA_ChannelReload(DMA_ADC_C_C_SEN_CHANNEL, C_VALUES_x_SIZE);
	DMA_ChannelEnable(DMA_ADC_C_C_SEN_CHANNEL, true);
	ADC_SamplingStart(ADC1);
	TIM_Reset(TIM6);
	TIM_Start(TIM6);
	LL_C_CStart(false);
	TIM_Start(TIM4);
}
//-----------------------------------------------

void CONTROL_VGS_SetResults(volatile RegulatorParamsStruct* Regulator)
{
	float Result = Regulator->VSenForm[Regulator->ConstantVFirstStep];
	if((Regulator->ConstantVFirstStep) != (Regulator->ConstantVLastStep))
	{
		for(Int16U i = Regulator->ConstantVFirstStep++; i < Regulator->ConstantVLastStep; i++)
			Result += Regulator->VSenForm[i];
		Result /= (Regulator->ConstantVLastStep - Regulator->ConstantVFirstStep);
		DataTable[REG_VGS] = (Int16U)Result;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		DataTable[REG_PROBLEM] = PROBLEM_NONE;
	}
	else
	{
		DataTable[REG_VGS] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
	}
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_IGES_SetResults(volatile RegulatorParamsStruct* Regulator)
{
	float Result = 0;
	if(Result > 0)
	{
		DataTable[REG_IGES] = (Int16U)Result;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		DataTable[REG_PROBLEM] = PROBLEM_NONE;
	}
	else
	{
		DataTable[REG_IGES] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_NEGATIVE_CURRENT;
	}
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_QG_SetResults()
{
	CONTROL_C_Processing();
	if(CONTROL_C_Start_Counter != CONTROL_C_Stop_Counter)
	{
		Int16U C_Counter = CONTROL_C_Stop_Counter - CONTROL_C_Start_Counter;
		float Result = 0;
		for(Int16U i = CONTROL_C_Start_Counter; i < CONTROL_C_Stop_Counter; i++)
			Result += MEASURE_C_CSenRaw[i];
		Result /= C_Counter;

		for(Int16U i = 0; i < C_VALUES_x_SIZE; i++)
		{
			CONTROL_C_CSenValues[i] = CU_C_ADCCToX(MEASURE_C_CSenRaw[i]);
		}
		Result = CU_C_ADCCToX((Int16U)Result);

		float Time = C_Counter * TIMER6_uS;
		float Qgate = Result * Time;

		DataTable[REG_QG_T] = (Int16U)(Time);
		DataTable[REG_QG_C] = (Int16U)(Result);
		DataTable[REG_QG] = (Int16U)(Qgate);
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		DataTable[REG_PROBLEM] = PROBLEM_NONE;
	}
	else
	{
		DataTable[REG_QG_T] = 0;
		DataTable[REG_QG_C] = 0;
		DataTable[REG_QG] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
	}

	CONTROL_C_Start_Counter = 0;
	CONTROL_C_Stop_Counter = 0;
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_C_Processing()
{
	CONTROL_C_Start_Counter = 0;
	CONTROL_C_Stop_Counter = 0;

	for(Int16U i = 0; i < C_VALUES_x_SIZE; i++)
	{
		if((CU_C_ADCCToX(MEASURE_C_CSenRaw[i]) > ((DataTable[REG_QG_C_THRESHOLD] / 100) * DataTable[REG_QG_C_SET])) && (CONTROL_C_Start_Counter == 0))
			CONTROL_C_Start_Counter = i;
		if((CU_C_ADCCToX(MEASURE_C_CSenRaw[C_VALUES_x_SIZE - i]) > ((DataTable[REG_QG_C_THRESHOLD] / 100) * DataTable[REG_QG_C_SET]))
				&& (CONTROL_C_Stop_Counter == 0))
			CONTROL_C_Stop_Counter = C_VALUES_x_SIZE - i;
	}
	CONTROL_C_Values_Counter = CONTROL_C_Stop_Counter;
}
//-----------------------------------------------

void CONTROL_V_StopProcess()
{
	TIM_Stop(TIM15);
	LL_V_VSetDAC(0);
	LL_V_ShortOut(true);
	LL_V_ShortPAU(true);
}
//------------------------------------------

void CONTROL_C_StopProcess()
{
	TIM_Stop(TIM6);
	TIM_Stop(TIM4);
	LL_C_CSetDAC(0);
	LL_C_CEnable(false);
	LL_C_CStart(false);
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, SS_None);
	DataTable[REG_FAULT_REASON] = Reason;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	CONTROL_State = NewState;
	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_STATE] = NewState;
	DataTable[REG_SUB_STATE] = NewSubState;
}
//------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------

