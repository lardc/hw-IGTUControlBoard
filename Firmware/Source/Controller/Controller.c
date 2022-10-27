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
#include "ExternalDAC.h"
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
volatile Int64U CONTROL_I_TimeCounter = 0;
volatile Int64U CONTROL_I_Start_Counter = 0;
volatile Int64U CONTROL_I_Stop_Counter = 0;
//
volatile Int64U	CONTROL_AfterPulsePause = 0;
volatile Int64U	CONTROL_BatteryChargeTimeCounter = 0;
volatile Int64U CONTROL_ConfigStateCounter = 0;
volatile Int16U CONTROL_Values_Counter = 0;
volatile Int16U CONTROL_I_Values_Counter = 0;
volatile Int16U CONTROL_UUValues[U_VALUES_x_SIZE];
volatile Int16U CONTROL_UUMeasValues[U_VALUES_x_SIZE];
volatile Int16U CONTROL_RegulatorOutput[U_VALUES_x_SIZE];
volatile Int16U CONTROL_RegulatorErr[U_VALUES_x_SIZE];
volatile Int16U CONTROL_DACRawData[U_VALUES_x_SIZE];
volatile Int16U CONTROL_UIMeasValues[U_VALUES_x_SIZE];
volatile Int16U CONTROL_IIGateValues[I_VALUES_x_SIZE];
//
float CONTROL_CurrentMaxValue = 0;
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
	Int16U EPIndexes[EP_COUNT] = {EP_U_U_FORM, EP_U_U_MEAS_FORM, EP_REGULATOR_OUTPUT, EP_REGULATOR_ERR, EP_U_DAC_RAW_DATA,
			EP_U_I_MEAS_FORM, EP_I_I_GATE_FORM};

	Int16U EPSized[EP_COUNT] =
			{U_VALUES_x_SIZE, U_VALUES_x_SIZE, U_VALUES_x_SIZE, U_VALUES_x_SIZE, U_VALUES_x_SIZE, U_VALUES_x_SIZE, I_VALUES_x_SIZE};

	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_I_Values_Counter};

	pInt16U EPDatas[EP_COUNT] = {(pInt16U)CONTROL_UUValues, (pInt16U)CONTROL_UUMeasValues,
			(pInt16U)CONTROL_RegulatorOutput, (pInt16U)CONTROL_RegulatorErr, (pInt16U)CONTROL_DACRawData,
			(pInt16U)CONTROL_UIMeasValues, (pInt16U)CONTROL_IIGateValues};

	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_SLAVE_NID, 0);

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

	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();
}
//------------------------------------------

void CONTROL_ResetIArray()
{
	for (Int16U i = 0; i < I_VALUES_x_SIZE; i++)
	{
		CONTROL_IIGateValues[i] = 0;
	}
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	LL_UShortOut(true);
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
	
	switch (ActionID)
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
			if (CONTROL_State == DS_Ready)
			{
				CONTROL_SetDeviceState(DS_InProcess, SS_PulsePrepare);
				CONTROL_UStartProcess();
			}
			else
				if (CONTROL_State == DS_InProcess)
					*pUserError = ERR_OPERATION_BLOCKED;
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_QG_START:
			if (CONTROL_State == DS_Ready)
			{
				CONTROL_SetDeviceState(DS_InProcess, SS_Pulse);
				CONTROL_IStartProcess();
			}
			else
				if (CONTROL_State == DS_InProcess)
					*pUserError = ERR_OPERATION_BLOCKED;
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_STOP_PROCESS:
			if (CONTROL_State == DS_InProcess)
			{
				CONTROL_UStopProcess();
				//CONTROL_IStopProcess();
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			break;

		case ACT_CLR_FAULT:
			if (CONTROL_State == DS_Fault)
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
		case SS_PulsePrepare:
			CONTROL_StartPrepare();
			CONTROL_SetDeviceState(DS_Ready, SS_Pulse);
			break;

		case SS_WaitAfterUPulse:
			CONTROL_USetResults(&RegulatorParams);
			break;

		case SS_WaitAfterIPulse:
			CONTROL_ISetResults();
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_UHighPriorityProcess()
{
	if(CONTROL_SubState == SS_Pulse)
	{
		if (MEASURE_UParams(&RegulatorParams))
							REGULATOR_UFormUpdate(&RegulatorParams);

		if(CONTROL_RegulatorCycle(&RegulatorParams))
		{
			CONTROL_UStopProcess();
			CONTROL_SetDeviceState(DS_InProcess, SS_WaitAfterUPulse);
		}
	}
}
//-----------------------------------------------

void CONTROL_IHighPriorityProcess()
{
	if(CONTROL_SubState == SS_Pulse)
	{
		TIM_Stop(TIM4);
		LL_IStart(true);
		LL_IISetDAC(0);
		TIM_Stop(TIM6);
		TIM_Reset(TIM6);
		ADC_SamplingStop(ADC1);
		DMA_TransferCompleteReset(DMA1, DMA_ISR_TCIF1);
		TIM_StatusClear(TIM4);
		//ExDAC_IUCutoff(0);
		//ExDAC_IUNegative(0);
		CONTROL_SetDeviceState(DS_InProcess, SS_WaitAfterIPulse);
	}
}
//-----------------------------------------------

bool CONTROL_RegulatorCycle(volatile RegulatorParamsStruct* Regulator)
{
	return REGULATOR_Process(Regulator);
}
//-----------------------------------------------

void CONTROL_USetResults(volatile RegulatorParamsStruct* Regulator)
{
	float Result = Regulator->UFormMeasured[Regulator->ConstantUFirstPulse];
	if ((Regulator->ConstantUFirstPulse) != (Regulator->ConstantULastPulse))
	{
		for (Int16U i = Regulator->ConstantUFirstPulse++; i < Regulator->ConstantULastPulse; i++)
			Result += Regulator->UFormMeasured[i];
		Result /= (Regulator->ConstantULastPulse - Regulator->ConstantUFirstPulse);
		DataTable[REG_U_VGS] = (Int16U)Result;
	}
	else
	{
		CONTROL_SetDeviceWarning(DW_CurrentNotReached);
		DataTable[REG_U_VGS] = 0;
	}
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_ISetResults()
{
	CONTROL_IProcessing();
	if (CONTROL_I_Start_Counter != CONTROL_I_Stop_Counter)
	{
		Int16U I_Counter = CONTROL_I_Stop_Counter - CONTROL_I_Start_Counter;
		float Result;
		for (Int16U i = CONTROL_I_Start_Counter; i < CONTROL_I_Stop_Counter; i++)
				Result += MEASURE_ADC_IGateRaw[i];
		Result /= I_Counter;

		for (Int16U i = 0; i < I_VALUES_x_SIZE; i++)
		{
			CONTROL_IIGateValues[i] = CU_IADCIToX(MEASURE_ADC_IGateRaw[i]);
		}
		Result = CU_IADCIToX((Int16U)Result);

		float Time = I_Counter * TIMER6_uS;
		float Qgate = Result * Time;

		DataTable[REG_I_T_IGATE] = (Int16U)(Time);
		DataTable[REG_I_AVERAGE_IGATE] = (Int16U)(Result);
		DataTable[REG_I_QG] = (Int16U)(Qgate);
	}
	else
	{
		CONTROL_SetDeviceWarning(DW_CurrentNotReached);
		DataTable[REG_I_T_IGATE] = 0;
		DataTable[REG_I_AVERAGE_IGATE] = 0;
		DataTable[REG_I_QG] = 0;
	}

	CONTROL_I_Start_Counter = 0;
	CONTROL_I_Stop_Counter = 0;
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------

void CONTROL_IProcessing()
{
	CONTROL_I_Start_Counter = 0;
	CONTROL_I_Stop_Counter = 0;

	for (Int16U i = 0; i < I_VALUES_x_SIZE; i++)
	{
		if ((CU_IADCIToX(MEASURE_ADC_IGateRaw[i]) > (0.3 * DataTable[REG_I_I_SET])) && (CONTROL_I_Start_Counter == 0))
				CONTROL_I_Start_Counter = i;
		if ((CU_IADCIToX(MEASURE_ADC_IGateRaw[I_VALUES_x_SIZE - i]) > (0.3 * DataTable[REG_I_I_SET])) && (CONTROL_I_Stop_Counter == 0))
				CONTROL_I_Stop_Counter = I_VALUES_x_SIZE - i;
	}
	CONTROL_I_Values_Counter = CONTROL_I_Stop_Counter;
}
//-----------------------------------------------

void CONTROL_StartPrepare()
{
	MEASURE_DMAIGateBufferClear();
	REGULATOR_CashVariables(&RegulatorParams);
	REGULATOR_UFormConfig(&RegulatorParams);
}
//-----------------------------------------------

void CONTROL_UStopProcess()
{
	TIM_Stop(TIM15);
	LL_UUSetDAC(0);
	LL_UShortOut(true);
}
//------------------------------------------

void CONTROL_UStartProcess()
{
	CONTROL_ResetOutputRegisters();
	LL_UShortOut(false);
	TIM_Reset(TIM15);
	TIM_Start(TIM15);
}
//-----------------------------------------------

void CONTROL_IStartProcess()
{
	CONTROL_ResetOutputRegisters();
	CONTROL_ResetIArray();
	//ADC_SwitchToHighSpeed();


	CONTROL_I_Values_Counter = 0;
	CONTROL_TimerMaxCounter = (Int16U)((float)DataTable[REG_I_T_CURRENT] / (float)TIMER4_uS);
	ExDAC_IUCutoff((float)DataTable[REG_I_U_CUTOFF]);
	ExDAC_IUNegative((float)DataTable[REG_I_U_NEGATIVE]);
	LL_IISetDAC(CU_IIToDAC((float)DataTable[REG_I_I_SET]));
	DELAY_US(5);
	CONTROL_I_TimeCounter = 0;
	TIM_Reset(TIM4);
	DMA_ChannelReload(DMA_ADC_I_GATE_CHANNEL, I_VALUES_x_SIZE);
	DMA_ChannelEnable(DMA_ADC_I_GATE_CHANNEL, true);
	ADC_SamplingStart(ADC1);
	TIM_Reset(TIM6);
	TIM_Start(TIM6);
	LL_IStart(false);
	TIM_Start(TIM4);
}
//-----------------------------------------------

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

void CONTROL_SetDeviceWarning(DeviceWarning NewWarning)
{
	DataTable[REG_WARNING] = NewWarning;
}
//------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------

