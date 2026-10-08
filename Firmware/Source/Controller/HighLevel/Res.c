// Header
//
#include "Res.h"

// Includes
//
#include "Board.h"
#include "Measurement.h"
#include "Controller.h"
#include "InitConfig.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "ConvertUtils.h"
#include "Global.h"
#include "Logging.h"
#include "PAU.h"
#include "Iges.h"

// Definitions
//
#define RES_PULSE_WIDTH_MS			10000	// мкс
#define RES_VG_FRONT_TIME			5000	// мкс
#define LINE_SHORT_CURRENT			5
#define LINE_RESISTANCE				4		// Ом
//
#define RES_PAU_SYNC_DELAY_US		500
#define RES_PAU_SYNC_DELAY_STEP		(Int16U)(RES_PAU_SYNC_DELAY_US / TIMER15_uS)

// Types
//
typedef enum __ResPulseKind
{
	RPK_Internal = 0,
	RPK_Pau
} ResPulseKind;

typedef enum __ResProcessState
{
	RPS_Ramp = 0,
	RPS_Plateau
} ResProcessState;

typedef enum __ResPauPrepareStage
{
	ResPau_Config = 0,
	ResPau_Waiting,
	ResPau_HW_Config
} ResPauPrepareStage;

// Variables
//
LogParamsStruct ResMeasureLog;
MeasureSample ResSampledData;
RingBuffersParams ResRingBuffers;
bool ResFineMeasure = false;
static float ResTestCurrent = V_I_R2_MAX, TargetVoltage;
static float ResPauVoltageAvg = 0;
static ResPulseKind ResPulse = RPK_Internal;
static ResProcessState ResState = RPS_Ramp;
static ResPauPrepareStage ResPauConfigStage = ResPau_Config;
static Int64U ResPauStateTimeout = 0;
static Int16U ResSamplesCounter = 0;

// Functions prototypes
//
void RES_CacheVariables();
static float RES_CalculateResistance(float Voltage, float Current_mA);
static void RES_ResetMeasureState();
static void RES_HandleFollowingError();
static void RES_HandleInternalComplete();
static void RES_PAUsyncProcess(bool State);

// Functions
//
void RES_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);
	REGULATOR_Mode(&RegulatorParams, (ResPulse == RPK_Pau) ? Parametric : FeedBack);
	LOG_ClearBuffers(&ResRingBuffers);

	TargetVoltage = DataTable[REG_RES_VOLTAGE] ? DataTable[REG_RES_VOLTAGE] : RES_TEST_VOLTAGE;

	RegulatorParams.dVg = TargetVoltage / (RES_VG_FRONT_TIME / TIMER15_uS);
	RegulatorParams.Counter = (ResPulse == RPK_Pau) ?
			INT16U_MAX : (RES_PULSE_WIDTH_MS / TIMER15_uS);

	ResMeasureLog.DataA = &ResSampledData.Voltage;
	ResMeasureLog.DataB = &ResSampledData.Current;
	ResMeasureLog.LogBufferA = &CONTROL_VoltageValues[0];
	ResMeasureLog.LogBufferB = &CONTROL_CurrentValues[0];
	ResMeasureLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	ResRingBuffers.DataA = &ResSampledData.Voltage;
	ResRingBuffers.DataB = &ResSampledData.Current;
	ResRingBuffers.RingCounterMask = LOG_COUNTER_MASK;

	ResState = RPS_Ramp;
	ResSamplesCounter = DataTable[REG_RES_PAU_SAMPLES];
}
//------------------------------

void RES_Prepare()
{
	ResPulse = RPK_Internal;

	Int16U CurrentRange = MEASURE_V_SetCurrentRange(ResTestCurrent);

	INITCFG_ConfigADC_VgsIges(CurrentRange);
	INITCFG_ConfigDMA_VgsIges();
	MEASURE_ResetDMABuffers();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	RES_CacheVariables();

	CONTROL_SetDeviceState(CONTROL_State, SS_ResProcess);
	CONTROL_StartHighPriorityProcesses();
}
//------------------------------

static float RES_CalculateResistance(float Voltage, float Current_mA)
{
	if(Current_mA == 0)
		return 0;

	return (Voltage - LINE_RESISTANCE * Current_mA / 1000) / Current_mA * 1000;
}
//-----------------------------------------------

static void RES_ResetMeasureState()
{
	ResFineMeasure = false;
	ResTestCurrent = V_I_R2_MAX;
	ResPulse = RPK_Internal;
	ResPauConfigStage = ResPau_Config;
}
//-----------------------------------------------

static void RES_HandleFollowingError()
{
	DataTable[REG_RES_RESULT] = 0;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
	RES_ResetMeasureState();

	if(ResSampledData.Current > LINE_SHORT_CURRENT)
	{
		DataTable[REG_PROBLEM] = PROBLEM_SHORT;
		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
		CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
}
//-----------------------------------------------

static void RES_HandleInternalComplete()
{
	MeasureSample AverageData = LOG_RingBufferGetAverage(&ResRingBuffers);

	if(!ResFineMeasure)
	{
		ResFineMeasure = true;
		ResTestCurrent = AverageData.Current;
		CONTROL_SetDeviceState(CONTROL_State, SS_ResPrepare);
	}
	else
	{
		ResFineMeasure = false;
		DataTable[REG_DBG_RES_I_MEAS_INTERNAL] = AverageData.Current;
		if(AverageData.Current < DataTable[REG_RES_I_THRESHOLD] && !DataTable[REG_PAU_EMULATED] && DataTable[REG_RES_I_THRESHOLD])
		{
			ResTestCurrent = AverageData.Current;
			ResPauConfigStage = ResPau_Config;
			DataTable[REG_DBG_RES_PAU_SWITCH] = true;
			CONTROL_SetDeviceState(DS_InProcess, SS_ResPauPrepare);
		}
		else
		{
			ResTestCurrent = V_I_R2_MAX;
			DataTable[REG_RES_RESULT] = RES_CalculateResistance(AverageData.Voltage, AverageData.Current);
			DataTable[REG_OP_RESULT] = OPRESULT_OK;
			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------------------------

static void RES_PAUsyncProcess(bool State)
{
	static Int16U SyncDelayCounter = 0;
	static bool SyncState = false;
	static Int16U ResSamplesLast = 0;
	static Int64U ResSamplesTimeoutCounter = 0;

	if(State)
	{
		if(SyncDelayCounter)
		{
			SyncDelayCounter--;

			if(!SyncDelayCounter)
			{
				PAU_SyncFlag = false;
				SyncState = true;
			}
		}
		else
		{
			if(PAU_SyncFlag)
			{
				ResSamplesCounter--;
				SyncDelayCounter = RES_PAU_SYNC_DELAY_STEP;
			}

			SyncState = false;
		}

		if(ResSamplesLast != ResSamplesCounter)
		{
			ResSamplesLast = ResSamplesCounter;
			ResSamplesTimeoutCounter = CONTROL_TimeCounter + PAU_SYNC_PERIOD_MAX;
		}
		else
		{
			if(CONTROL_TimeCounter > ResSamplesTimeoutCounter)
			{
				CONTROL_StopHighPriorityProcesses();
				RES_ResetMeasureState();
				CONTROL_SwitchToFault(DF_PAU_SYNC_TIMEOUT);
			}
		}
	}
	else
	{
		ResSamplesLast = 0;
		SyncDelayCounter = RES_PAU_SYNC_DELAY_STEP;
		SyncState = false;
	}

	LL_SyncPAU(SyncState);
}
//-----------------------------------------------

void RES_Process()
{
	ResSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&ResRingBuffers);
	LOG_LoggingData(&ResMeasureLog);

	RegulatorParams.SampledData = ResSampledData.Voltage;

	switch(ResState)
	{
		case RPS_Ramp:
			if(ResPulse == RPK_Pau)
				RES_PAUsyncProcess(false);

			if(RegulatorParams.Target < TargetVoltage)
				RegulatorParams.Target += RegulatorParams.dVg;
			else
			{
				RegulatorParams.Target = TargetVoltage;
				LL_SyncOSC(true);
				IsImpulse = true;
				ResState = RPS_Plateau;

				if(ResPulse == RPK_Pau)
					PAU_ShortInput(false);
			}

			if(REGULATOR_Process(&RegulatorParams))
			{
				CONTROL_StopHighPriorityProcesses();

				if(RegulatorParams.FollowingError)
					RES_HandleFollowingError();
				else if(ResPulse == RPK_Internal)
					RES_HandleInternalComplete();
				else
				{
					ResPauVoltageAvg = LOG_RingBufferGetAverage(&ResRingBuffers).Voltage;
					ResPauStateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
					CONTROL_SetDeviceState(DS_InProcess, SS_ResPauSaveResult);
				}
			}
			break;

		case RPS_Plateau:
			if(ResPulse == RPK_Internal)
			{
				if(REGULATOR_Process(&RegulatorParams))
				{
					CONTROL_StopHighPriorityProcesses();

					if(RegulatorParams.FollowingError)
						RES_HandleFollowingError();
					else
						RES_HandleInternalComplete();
				}
			}
			else
			{
				RES_PAUsyncProcess(true);

				REGULATOR_Process(&RegulatorParams);

				if(RegulatorParams.FollowingError)
				{
					CONTROL_StopHighPriorityProcesses();
					RES_HandleFollowingError();
				}
				else if(!ResSamplesCounter)
				{
					CONTROL_StopHighPriorityProcesses();
					ResPauVoltageAvg = LOG_RingBufferGetAverage(&ResRingBuffers).Voltage;
					ResPauStateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
					CONTROL_SetDeviceState(DS_InProcess, SS_ResPauSaveResult);
				}
			}
			break;
	}
}
//-----------------------------------------------

void RES_PauPrepare()
{
	float PAU_Range = 0;

	switch(ResPauConfigStage)
	{
		case ResPau_Config:
			if(PAU_IsReady() || PAU_IsConfigReady())
			{
				PAU_Range = PAU_SelectRangeByCurrent(ResTestCurrent);

				if(PAU_Configure(PAU_CHANNEL_IGTU, PAU_Range, DataTable[REG_RES_PAU_SAMPLES]))
				{
					ResPauStateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
					ResPauConfigStage = ResPau_Waiting;
				}
				else
				{
					ResPauConfigStage = ResPau_Config;
					RES_ResetMeasureState();
					CONTROL_SwitchToFault(DF_PAU_INTERFACE);
				}
			}
			else
			{
				ResPauConfigStage = ResPau_Config;
				RES_ResetMeasureState();
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			}
			break;

		case ResPau_Waiting:
			if(PAU_IsConfigReady())
				ResPauConfigStage = ResPau_HW_Config;
			else if(CONTROL_TimeCounter >= ResPauStateTimeout)
			{
				ResPauConfigStage = ResPau_Config;
				RES_ResetMeasureState();
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			}
			break;

		case ResPau_HW_Config:
			ResPauConfigStage = ResPau_Config;
			ResPulse = RPK_Pau;

			Int16U CurrentRange = MEASURE_V_SetCurrentRange(ResTestCurrent);

			INITCFG_ConfigADC_VgsIges(CurrentRange);
			INITCFG_ConfigDMA_VgsIges();
			MEASURE_ResetDMABuffers();

			LL_V_ShortOut(false);
			PAU_ShortInput(true);
			CONTROL_SwitchOutMUX(Voltage);

			RES_CacheVariables();

			CONTROL_SetDeviceState(DS_InProcess, SS_ResProcess);
			CONTROL_StartHighPriorityProcesses();
			break;
	}
}
//-----------------------------------------------

void RES_PauSaveResult()
{
	float PauCurrent = 0;

	if(PAU_IsReady())
	{
		if(PAU_ReadMeasuredData(&PauCurrent))
		{
			DataTable[REG_RES_RESULT] = RES_CalculateResistance(ResPauVoltageAvg, PauCurrent);
			DataTable[REG_OP_RESULT] = OPRESULT_OK;
			ResTestCurrent = V_I_R2_MAX;
			ResPulse = RPK_Internal;
			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
		else
		{
			RES_ResetMeasureState();
			CONTROL_SwitchToFault(DF_PAU_INTERFACE);
		}
		DataTable[REG_DBG_RES_I_MEAS_PAU] = PauCurrent;
	}
	else if(CONTROL_TimeCounter >= ResPauStateTimeout)
	{
		RES_ResetMeasureState();
		CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
	}
}
//-----------------------------------------------
