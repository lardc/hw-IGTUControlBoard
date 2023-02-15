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
#include "ConvertUtils.h"
#include "Regulator.h"

// Types
//
typedef enum __CommutationState
{
	Current = 0,
	Voltage = 1
} CommutationState;
//

typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
volatile Int64U CONTROL_TimeCounter = 0;
static Boolean CycleActive = false;
//
volatile float CONTROL_RegulatorOutputValues[VALUES_x_SIZE];
volatile float CONTROL_RegulatorErrValues[VALUES_x_SIZE];
volatile float CONTROL_VoltageValues[VALUES_x_SIZE];
volatile float CONTROL_CurrentValues[VALUES_x_SIZE];
//
volatile Int16U CONTROL_RegulatorValues_Counter = 0;
volatile Int16U CONTROL_Values_Counter = 0;
//
float TrigCurrentLow = 0, TrigCurrentHigh = 0;



Int16U CONTROL_TOCUHPState;
Boolean CONTROL_SelfMode = false;
volatile Int16U CONTROL_TimerMaxCounter = 0;
volatile Int64U CONTROL_C_TimeCounter = 0;
volatile Int64U CONTROL_C_Start_Counter = 0;
volatile Int64U CONTROL_C_Stop_Counter = 0;
//


/// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
void CONTROL_ResetHardwareToDefaultState();
void CONTROL_LogicProcess();
void CONTROL_SwitchOutMUX(CommutationState Commutation);
void CONTROL_CacheVariables();
void CONTROL_VgsProcess(MeasureSample SampledData);
void CONTROL_ResetOutputRegisters();
void CONTROL_CacheVariables();

// Functions
//
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[FEP_COUNT] = {EP_VOLTAGE, EP_CURRENT, EP_REGULATOR_ERR, EP_REGULATOR_OUTPUT};
	Int16U EPSized[FEP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE, VALUES_x_SIZE, VALUES_x_SIZE};
	pInt16U EPCounters[FEP_COUNT] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_RegulatorValues_Counter, (pInt16U)&CONTROL_RegulatorValues_Counter};

	pFloat32 EPDatas[FEP_COUNT] = {(pFloat32)CONTROL_VoltageValues, (pFloat32)CONTROL_CurrentValues,
			(pFloat32)CONTROL_RegulatorErrValues, (pFloat32)CONTROL_RegulatorOutputValues};

	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, CAN_NID);

	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitFEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Сброс значений
	DEVPROFILE_ResetControlSection();

	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	CONTROL_ResetHardwareToDefaultState();

	CONTROL_SetDeviceState(DS_None, SS_None);
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

void CONTROL_ResetHardwareToDefaultState()
{
	LL_SyncTOCUHP(false);
	LL_SyncOSC(false);
	LL_SyncPAU(false);
	LL_V_ShortOut(true);
	LL_V_ShortPAU(true);
	LL_V_Diagnostic(false);
	LL_C_Diagnostic(false);
	LL_C_CStart(false);
	LL_C_CEnable(false);
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_LogicProcess();

	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
				CONTROL_SetDeviceState(DS_Ready, SS_None);
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
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_VgsPrepare);
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
				CONTROL_ForceStopProcess();

				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				DataTable[REG_PROBLEM] = PROBLEM_FORCED_STOP;

				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			break;

		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
				CONTROL_ResetToDefaultState();
			break;

		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		default:
			if(CONTROL_State == DS_None)
				return DIAG_HandleDiagnosticAction(ActionID, pUserError);
			else
				*pUserError = ERR_OPERATION_BLOCKED;
	}
	return true;
}
//-----------------------------------------------

void CONTROL_LogicProcess()
{
	switch(CONTROL_SubState)
	{
	case SS_VgsPrepare:
		(DataTable[REG_VGS_I_TRIG] < DataTable[REG_V_I_SENS_THRESHOLD]) ? LL_V_CoefCSensLowRange() : LL_V_CoefCSensHighRange();

		LL_V_CLimitHighRange();
		LL_V_ShortOut(false);
		LL_V_ShortPAU(true);
		CONTROL_SwitchOutMUX(Voltage);

		CONTROL_CacheVariables();
		REGULATOR_CacheVgsVariables(&RegulatorParams);

		CONTROL_SetDeviceState(DS_Ready, SS_VgsPulse);
		CONTROL_V_StartProcess();
		break;

		case SS_VgsSaveResults:
			CONTROL_VGS_SaveResults(&RegulatorParams);
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

void CONTROL_SwitchOutMUX(CommutationState Commutation)
{
	static CommutationState LastCommutation = Current;

	if(LastCommutation != Commutation)
	{
		if(Commutation == Current)
			LL_OutMultiplexCurrent();
		else
			LL_OutMultiplexVoltage();

		LastCommutation = Commutation;

		DELAY_MS(20);
	}
}
//-----------------------------------------------

void CONTROL_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheCommonVariables(&RegulatorParams);

	TrigCurrentHigh = DataTable[REG_VGS_I_TRIG];
	TrigCurrentLow = DataTable[REG_VGS_I_TRIG] - DataTable[REG_VGS_I_TRIG] * DataTable[REG_VGS_dI_TRIG] / 100;
}
//-----------------------------------------------

void CONTROL_HighPriorityProcess()
{
	MeasureSample SampledData = MEASURE_SampleVgsIges();

	switch(CONTROL_SubState)
	{
		case SS_VgsPulse:
			CONTROL_VgsProcess(SampledData);
			break;

		case SS_IgesPulse:
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_VgsProcess(MeasureSample SampledData)
{
	if(SampledData.Current >= TrigCurrentLow)
		RegulatorParams.dVg = DataTable[REG_VGS_SLOW_RATE] * TIMER15_uS;

	if(SampledData.Current < TrigCurrentHigh)
	{
		if(RegulatorParams.Target < DataTable[REG_VGS_V_MAX])
			RegulatorParams.Target += RegulatorParams.dVg;
		else
			RegulatorParams.Target = DataTable[REG_VGS_V_MAX];

		RegulatorParams.SampledData = SampledData.Voltage;
	}
	else
	{
		CONTROL_V_StopProcess();
		CONTROL_SetDeviceState(CONTROL_State, SS_VgsSaveResults);
	}

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_V_StopProcess();

		DataTable[REG_VGS] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;

		if(RegulatorParams.FollowingError)
		{
			if(SampledData.Current > TrigCurrentHigh)
			{
				DataTable[REG_PROBLEM] = PROBLEM_GATE_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}

		if(SampledData.Current < TrigCurrentHigh)
		{
			DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------------------------

void CONTROL_V_StopProcess()
{
	TIM_Stop(TIM15);
	LL_V_VSetDAC(0);

	CONTROL_ResetHardwareToDefaultState();
}
//------------------------------------------

void CONTROL_V_StartProcess()
{
	LL_SyncOSC(true);

	TIM_Reset(TIM15);
	TIM_Start(TIM15);
}
//-----------------------------------------------

void CONTROL_VGS_SaveResults(volatile RegulatorParamsStruct* Regulator)
{
	float Sum = 0;

	for(Int16U i = 0; i < REGULATOR_RING_BUFFER_SIZE; i++)
		Sum += Regulator->RingBuffer[i];

	DataTable[REG_VGS] = Sum / REGULATOR_RING_BUFFER_SIZE;
	DataTable[REG_OP_RESULT] = OPRESULT_OK;
}
//-----------------------------------------------

void CONTROL_ForceStopProcess()
{
	CONTROL_C_StopProcess();
	CONTROL_V_StopProcess();

	CONTROL_ResetHardwareToDefaultState();
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













void CONTROL_C_ResetArray()
{
	/*for(Int16U i = 0; i < C_VALUES_x_SIZE; i++)
	{
		CONTROL_C_CSenValues[i] = 0;
	}*/
}
//------------------------------------------




void CONTROL_IGES_StartProcess()
{
	/*LL_OutMultiplexVoltage();
	LL_Indication(true);
	LL_V_CLimitLowRange();
	DELAY_MS(5);

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
		DataTable[REG_PROBLEM] = DF_PAU_REQUEST_ERROR;
	}*/
}
//-----------------------------------------------



void CONTROL_QG_StartProcess()
{
	/*LL_OutMultiplexCurrent();
	DELAY_MS(5);
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
		//DMA_ChannelReload(DMA_ADC_C_SEN_CHANNEL, C_VALUES_x_SIZE);
		//DMA_ChannelEnable(DMA_ADC_C_SEN_CHANNEL, true);
		ADC_SamplingStart(ADC1);
		TIM_Reset(TIM6);
		TIM_Start(TIM6);
		LL_C_CStart(true);
		TIM_Start(TIM4);
		LL_SyncOSC(true);
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = DF_TOCUHP_REQUEST_ERROR;
		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}*/
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
			DataTable[REG_PROBLEM] = DF_NEGATIVE_CURRENT;
		}
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = DF_PAU_REQUEST_ERROR;
	}
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_QG_SetResults()
{
	/*CONTROL_C_Processing();
	if(TOCUHP_IsInFaultOrDisabled())
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = DF_TOCUHP_FAULT;
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
		DataTable[REG_PROBLEM] = DF_CURRENT_NOT_REACHED;
	}

	CONTROL_C_Start_Counter = 0;
	CONTROL_C_Stop_Counter = 0;*/
}
//-----------------------------------------------

void CONTROL_C_Processing()
{
	/*CONTROL_C_Start_Counter = 0;
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
	}*/
}
//-----------------------------------------------



void CONTROL_C_StopProcess()
{
	/*LL_SyncTOCUHP(false);
	TIM_Stop(TIM6);
	TIM_StatusClear(TIM6);
	TIM_Stop(TIM4);
	TIM_StatusClear(TIM4);
	LL_SyncOSC(false);
	LL_C_CSetDAC(0);
	LL_C_CStart(false);
	LL_C_CEnable(false);
	LL_ExDACVCutoff(0);
	LL_ExDACVNegative(0);
	ADC_SamplingStop(ADC1);
	DMA_TransferCompleteReset(DMA1, DMA_ISR_TCIF1);
	LL_Indication(false);*/
}
//------------------------------------------



