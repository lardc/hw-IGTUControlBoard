// Header
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
#include "PAU.h"
#include "TOCUHP.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
Int16U CONTROL_TOCUHPState;
static Boolean CycleActive = false;
//
Boolean CONTROL_SelfMode = false;
volatile Int64U CONTROL_TimeCounter = 0;
volatile Int16U CONTROL_TimerMaxCounter = 0;
volatile Int64U CONTROL_C_TimeCounter = 0;
volatile Int64U CONTROL_C_Start_Counter = 0;
volatile Int64U CONTROL_C_Stop_Counter = 0;
//
//
volatile Int16U CONTROL_V_Values_Counter = 0;
volatile Int16U CONTROL_C_Values_Counter = 0;
volatile Int16U CONTROL_V_VValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_VSenValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_RegErrValues[V_VALUES_x_SIZE];
volatile Int16U CONTROL_V_CSenValues[V_VALUES_x_SIZE];
//
volatile float CONTROL_C_CSenValues[C_VALUES_x_SIZE];
volatile float CONTROL_C_VSenValues[C_VALUES_x_SIZE];
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
	Int16U EPIndexes[EP_COUNT] = {EP_V_V_FORM, EP_V_V_MEAS_FORM, EP_REGULATOR_ERR,
	EP_V_C_MEAS_FORM, EP_C_C_FORM};

	Int16U EPSized[EP_COUNT] = {V_VALUES_x_SIZE, V_VALUES_x_SIZE, V_VALUES_x_SIZE, V_VALUES_x_SIZE,
	C_VALUES_x_SIZE};

	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter,
			(pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter, (pInt16U)&CONTROL_V_Values_Counter,
			(pInt16U)&CONTROL_C_Values_Counter};

	pInt16U EPDatas[EP_COUNT] = {(pInt16U)CONTROL_V_VValues, (pInt16U)CONTROL_V_VSenValues,
			(pInt16U)CONTROL_V_RegErrValues, (pInt16U)CONTROL_V_CSenValues, (pInt16U)CONTROL_C_CSenValues};

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

	CONTROL_MonitorSafety();

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
				CONTROL_SetDeviceState(DS_None, SS_None);
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
				CONTROL_SetDeviceState(DS_InProcess, SS_IgesPulse);
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

void CONTROL_MonitorSafety()
{
	bool SystemIsSafe = CONTROL_GetSafetyState();

	if(CONTROL_State == DS_InProcess)
	{
		if(!SystemIsSafe)
		{
			CONTROL_ForceResetHardware();
			CONTROL_SetDeviceState(DS_Ready, SS_None);
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
			DataTable[REG_PROBLEM] = PROBLEM_SAFETY_VIOLATION;
		}
	}
}
//-----------------------------------------------

bool CONTROL_GetSafetyState()
{
	bool SafetyInput = LL_SafetyState();
	DataTable[REG_SAFETY_STATE] = SafetyInput ? 1 : 0;

	if(DataTable[REG_MUTE_SAFETY_MONITOR])
		return true;
	else
		return SafetyInput;
}
//-----------------------------------------------

void CONTROL_LogicProcess()
{
	switch(CONTROL_SubState)
	{
		case SS_VgsWaitAfterPulse:
			CONTROL_VGS_SetResults(&RegulatorParams);
			if(CONTROL_State == DS_Selftest)
				DIAG_V_SelfTestFinished();
			CONTROL_SetDeviceState(DS_Ready, SS_None);
			break;

		case SS_IgesWaitAfterPulse:
			CONTROL_IGES_SetResults(&RegulatorParams);
			break;

		case SS_QgWaitAfterPulse:
			CONTROL_QG_SetResults();
			if(CONTROL_State == DS_Selftest)
				DIAG_C_SelfTestFinished();
			CONTROL_SetDeviceState(DS_Ready, SS_None);
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
			if(MEASURE_VGS_Params(&RegulatorParams, CONTROL_State))
				REGULATOR_VGS_FormUpdate(&RegulatorParams);

			if(CONTROL_RegulatorCycle(&RegulatorParams))
			{
				CONTROL_V_StopProcess();
				CONTROL_SetDeviceState(CONTROL_State, SS_VgsWaitAfterPulse);
			}
			break;

		case SS_IgesPulse:
			MEASURE_IGES_Params(&RegulatorParams, CONTROL_State);
			LL_SyncPAU(false);
			if(REGULATOR_IGES_SyncPAU(&RegulatorParams))
				LL_SyncPAU(true);
			if(CONTROL_RegulatorCycle(&RegulatorParams))
			{
				CONTROL_V_StopProcess();
				CONTROL_SetDeviceState(CONTROL_State, SS_IgesWaitAfterPulse);
			}
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_C_HighPriorityProcess(bool IsInProgress)
{
	if(CONTROL_SubState == SS_QgPulse)
	{
		if(IsInProgress)
		{
			CONTROL_C_CSenValues[CONTROL_C_Values_Counter] = MEASURE_C_DMAExtractCSen();
			CONTROL_C_VSenValues[CONTROL_C_Values_Counter] = MEASURE_C_DMAExtractVSen();
			MEASURE_C_DMABufferClear();
			CONTROL_C_Values_Counter++;
		}
		else
		{
			CONTROL_C_StopProcess();
			CONTROL_SetDeviceState(CONTROL_State, SS_QgWaitAfterPulse);
		}
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
	LL_Indication(true);
	LL_V_CLimitHighRange();
	if(DataTable[REG_VGS_C_TRIG] <= DataTable[REG_V_C_SENS_THRESHOLD])
		LL_V_CoefCSensLowRange();
	else
		LL_V_CoefCSensHighRange();
	REGULATOR_VGS_FormConfig(&RegulatorParams);
	LL_V_ShortPAU(true);
	LL_V_ShortOut(false);
	CONTROL_V_StartProcess();
}
//-----------------------------------------------

void CONTROL_IGES_StartProcess()
{
	LL_Indication(true);
	LL_V_CLimitLowRange();
	if(PAU_Configure(PAU_CHANNEL_IGTU, PAU_AUTO_RANGE, DataTable[REG_IGES_T_V_CONSTANT]))
	{
		REGULATOR_IGES_FormConfig(&RegulatorParams);
		LL_V_ShortPAU(false);
		LL_V_ShortOut(false);
		CONTROL_V_StartProcess();
	}
	else
	{
		CONTROL_V_StopProcess();
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_PAU_REQUEST_ERROR;
	}
}
//-----------------------------------------------

void CONTROL_V_StartProcess()
{
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
	if((TOCUHP_ReadState(&CONTROL_TOCUHPState))
			&& (TOCUHP_Configure(DataTable[REG_QG_V_POWER], DataTable[REG_QG_V_POWER])))
	{
		LL_Indication(true);
		LL_SyncTOCUHP(true);
		LL_ExDACVCutoff((float)DataTable[REG_QG_V_CUTOFF]);
		LL_ExDACVNegative((float)DataTable[REG_QG_V_NEGATIVE]);
		LL_C_CSetDAC(CU_C_CToDAC((float)DataTable[REG_QG_C_SET]));
		DELAY_US(5);
		CONTROL_C_TimeCounter = 0;
		TIM_Reset(TIM4);
		DMA_ChannelReload(DMA_ADC_C_SEN_CHANNEL, C_VALUES_x_SIZE);
		DMA_ChannelEnable(DMA_ADC_C_SEN_CHANNEL, true);
		ADC_SamplingStart(ADC1);
		TIM_Reset(TIM6);
		TIM_Start(TIM6);
		LL_C_CStart(true);
		TIM_Start(TIM4);
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_TOCUHP_REQUEST_ERROR;
		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
}
//-----------------------------------------------

void CONTROL_VGS_SetResults(volatile RegulatorParamsStruct* Regulator)
{
	float Vgs = Regulator->CTrigVSen;
	if((Regulator->ConstantVFirstStep) != (Regulator->ConstantVLastStep))
	{
		for(Int16U i = Regulator->ConstantVFirstStep++; i < Regulator->ConstantVLastStep; i++)
			Vgs += Regulator->VSenForm[i];
		Vgs /= (Regulator->ConstantVLastStep - Regulator->ConstantVFirstStep);
		DataTable[REG_VGS] = Vgs;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		DataTable[REG_PROBLEM] = PROBLEM_NONE;
	}
	else
	{
		DataTable[REG_VGS] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
	}
}
//-----------------------------------------------

void CONTROL_IGES_SetResults(volatile RegulatorParamsStruct* Regulator)
{
	float Iges = 0;
	if(PAU_ReadMeasuredData(&Iges))
	{
		if(Iges > 0)
		{
			DataTable[REG_IGES] = (Int16U)Iges;
			DataTable[REG_OP_RESULT] = OPRESULT_OK;
			DataTable[REG_PROBLEM] = PROBLEM_NONE;
		}
		else
		{
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
			DataTable[REG_PROBLEM] = PROBLEM_NEGATIVE_CURRENT;
		}
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_PAU_REQUEST_ERROR;
	}
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_QG_SetResults()
{
	CONTROL_C_Processing();
	if(TOCUHP_IsInFaultOrDisabled())
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_TOCUHP_FAULT;
	}
	else if(CONTROL_C_Start_Counter != CONTROL_C_Stop_Counter)
	{
		Int16U C_Counter = CONTROL_C_Stop_Counter - CONTROL_C_Start_Counter;
		float C = 0;
		for(Int16U i = CONTROL_C_Start_Counter; i < CONTROL_C_Stop_Counter; i++)
			C += CONTROL_C_CSenValues[i];
		C /= C_Counter;

		float Time = C_Counter * TIMER6_uS;
		float Qgate = C * Time;

		DataTable[REG_QG_T] = (Int16U)(Time);
		DataTable[REG_QG_C] = (Int16U)(C);
		DataTable[REG_QG] = (Int16U)(Qgate);
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		DataTable[REG_PROBLEM] = PROBLEM_NONE;
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
	}

	CONTROL_C_Start_Counter = 0;
	CONTROL_C_Stop_Counter = 0;
}
//-----------------------------------------------

void CONTROL_C_Processing()
{
	CONTROL_C_Start_Counter = 0;
	CONTROL_C_Stop_Counter = 0;

	for(Int16U i = 0; i < C_VALUES_x_SIZE; i++)
	{
		CONTROL_C_CSenValues[i] = CU_C_ADCCToX(CONTROL_C_CSenValues[i]);
		CONTROL_C_VSenValues[i] = CU_C_ADCVToX(CONTROL_C_VSenValues[i]);
		if(((CONTROL_C_CSenValues[i]) > ((DataTable[REG_QG_C_THRESHOLD] / 100) * DataTable[REG_QG_C_SET]))
				&& (CONTROL_C_Start_Counter == 0))
			CONTROL_C_Start_Counter = i;
		if(((CONTROL_C_CSenValues[C_VALUES_x_SIZE - i])
				> ((DataTable[REG_QG_C_THRESHOLD] / 100) * DataTable[REG_QG_C_SET])) && (CONTROL_C_Stop_Counter == 0))
			CONTROL_C_Stop_Counter = C_VALUES_x_SIZE - i;
	}
}
//-----------------------------------------------

void CONTROL_V_StopProcess()
{
	TIM_Stop(TIM15);
	TIM_StatusClear(TIM15);
	LL_V_VSetDAC(0);
	LL_V_ShortOut(true);
	LL_V_ShortPAU(true);
	LL_Indication(false);
}
//------------------------------------------

void CONTROL_C_StopProcess()
{
	LL_SyncTOCUHP(false);
	TIM_Stop(TIM6);
	TIM_StatusClear(TIM6);
	TIM_Stop(TIM4);
	TIM_StatusClear(TIM4);
	LL_C_CSetDAC(0);
	LL_C_CStart(false);
	LL_C_CEnable(false);
	LL_ExDACVCutoff(0);
	LL_ExDACVNegative(0);
	ADC_SamplingStop(ADC1);
	DMA_TransferCompleteReset(DMA1, DMA_ISR_TCIF1);
	LL_Indication(false);
}
//------------------------------------------

void CONTROL_ForceResetHardware()
{
	CONTROL_C_StopProcess();
	CONTROL_V_StopProcess();
}

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

