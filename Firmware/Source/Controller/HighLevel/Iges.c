// Header
//
#include "Iges.h"

// Includes
//
#include "PAU.h"
#include "DataTable.h"
#include "LowLevel.h"
#include "ConvertUtils.h"
#include "Regulator.h"
#include "InitConfig.h"
#include "Logging.h"
#include "Measurement.h"
#include "DeviceObjectDictionary.h"
#include "Delay.h"

// Definitions
//
#define PAU_SYNC_DELAY_US				500
#define PAU_SYNC_DELAY_STEP				(Int16U)(PAU_SYNC_DELAY_US / TIMER15_uS)
//
#define PAU_UDATE_STATE_PERIOD			50	// in ms

// Types
//
typedef enum __IgesPrepareStage
{
	PAU_Config = 0,
	PAU_Wating = 1,
	HW_Config = 2
} IgesPrepareStage;

// Variables
//
LogParamsStruct IgesLog;
MeasureSample IgesSampledData;
RingBuffersParams IgesRingBuffers;
IgesPrepareStage ConfigStage = PAU_Config;
Int64U PAU_StateTimeout = 0;
bool PAU_SyncFlag = false;
Int16U IgesSamplesCounter = 0;

// Function prototypes
//
void IGES_CacheVariables();
void IGES_PAUsyncProcess(bool State);

// Functions
//
void IGES_Prepare()
{
	Int16U PAU_State;
	float PAU_Current = 0;

	if(!PAU_UpdateState(&PAU_State))
		CONTROL_SwitchToFault(DF_PAU_INTERFACE);

	if(DataTable[REG_PAU_EMULATED])
		ConfigStage = HW_Config;

	switch(ConfigStage)
	{
		case PAU_Config:
			if(PAU_State == PS_Ready || PAU_State == PS_ConfigReady)
			{
				switch((Int16U)DataTable[REG_IGES_RANGE])
				{
					case PAU_CODE_RANGE0:
						PAU_Current = PAU_I_RANGE_0;
						break;

					case PAU_CODE_RANGE1:
						PAU_Current = PAU_I_RANGE_1;
						break;

					case PAU_CODE_RANGE2:
						PAU_Current = PAU_I_RANGE_2;
						break;
				}

				if(PAU_Configure(PAU_CHANNEL_IGTU, PAU_Current, DataTable[REG_IGES_SAMPLES_NUMBER]))
				{
					PAU_StateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
					ConfigStage = PAU_Wating;
				}
				else
				{
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SwitchToFault(DF_PAU_INTERFACE);
				}
			}
			else
			{
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			}
			break;

		case PAU_Wating:
			if(PAU_State == PS_ConfigReady)
				ConfigStage = HW_Config;
			else
			{
				if(CONTROL_TimeCounter >= PAU_StateTimeout)
				{
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
				}
			}
			break;

		case HW_Config:
			ConfigStage = PAU_Config;

			Int16U CurrentRange = MEASURE_V_SetCurrentRange(THRESHOLD_V_I_R0);

			INITCFG_ConfigADC_VgsIges(CurrentRange);
			INITCFG_ConfigDMA_VgsIges();

			LL_V_ShortOut(false);
			PAU_ShortInput(true);
			CONTROL_SwitchOutMUX(Voltage);
			REGULATOR_Mode(&RegulatorParams, Parametric);

			IGES_CacheVariables();

			CONTROL_SetDeviceState(DS_InProcess, SS_IgesProcess);
			CONTROL_StartHighPriorityProcesses();
			break;
	}
}
//-----------------------------------------------

void IGES_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);

	RegulatorParams.dVg = DataTable[REG_IGES_V_RATE] * TIMER15_uS;

	IgesLog.DataA = &IgesSampledData.Voltage;
	IgesLog.DataB = &IgesSampledData.Current;
	IgesLog.LogBufferA = &CONTROL_VoltageValues[0];
	IgesLog.LogBufferB = &CONTROL_CurrentValues[0];
	IgesLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	IgesRingBuffers.DataA = &IgesSampledData.Voltage;
	IgesRingBuffers.DataB = &IgesSampledData.Current;
	IgesRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
	//
	IgesSamplesCounter = DataTable[REG_IGES_SAMPLES_NUMBER];
}
//-----------------------------------------------

void IGES_Process()
{
	static bool StartPulsePlate = false;

	IgesSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&IgesRingBuffers);
	LOG_LoggingData(&IgesLog);

	if(RegulatorParams.Target < DataTable[REG_IGES_V])
		RegulatorParams.Target += RegulatorParams.dVg;
	else
	{
		if(!StartPulsePlate)
		{
			StartPulsePlate = true;
			RegulatorParams.Target = DataTable[REG_IGES_V];
			PAU_ShortInput(false);
		}
	}

	IGES_PAUsyncProcess(StartPulsePlate);

	RegulatorParams.SampledData = IgesSampledData.Voltage;

	REGULATOR_Process(&RegulatorParams);

	if(!IgesSamplesCounter)
	{
		StartPulsePlate = false;
		CONTROL_StopHighPriorityProcesses();

		if(RegulatorParams.FollowingError)
		{
			if(IgesSampledData.Current > VGS_CURRENT_MAX)
			{
				DataTable[REG_PROBLEM] = PROBLEM_GATE_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);

			DataTable[REG_IGES_RESULT] = 0;
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		}

		PAU_StateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
		CONTROL_SetDeviceState(DS_InProcess, SS_IgesSaveResult);
	}
}
//-----------------------------------------------

void IGES_PAUsyncProcess(bool State)
{
	static Int16U SyncDelayCounter = 0;
	static bool SyncState = false;

	if(State)
	{
		if(DataTable[REG_PAU_EMULATED])
		{
			IgesSamplesCounter--;
			return;
		}

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
				IgesSamplesCounter--;
				SyncDelayCounter = PAU_SYNC_DELAY_STEP;
			}

			SyncState = false;
		}
	}
	else
	{
		SyncDelayCounter = PAU_SYNC_DELAY_STEP;
		SyncState = false;
	}

	LL_SyncPAU(SyncState);
}
//-----------------------------------------------

void IGES_SaveResults()
{
	float Iges = 0;
	Int16U PAU_State;

	if(!PAU_UpdateState(&PAU_State))
		CONTROL_SwitchToFault(DF_PAU_INTERFACE);

	if(DataTable[REG_PAU_EMULATED])
	{
		DataTable[REG_IGES_RESULT] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;

		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
	{
		if(PAU_State == PS_Ready)
		{
			if(PAU_ReadMeasuredData(&Iges))
			{
				if(Iges < MEASURE_IGES_CURRENT_MIN)
					DataTable[REG_WARNING] = WARNING_IGES_TOO_LOW;
				if(Iges > MEASURE_IGES_CURRENT_MAX)
					DataTable[REG_WARNING] = WARNING_IGES_TOO_HIGH;

				DataTable[REG_IGES_RESULT] = Iges;
				DataTable[REG_OP_RESULT] = OPRESULT_OK;

				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
			{
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_PAU_INTERFACE);
			}
		}
		else
		{
			if(CONTROL_TimeCounter >= PAU_StateTimeout)
			{
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			}
		}
	}
}
//-----------------------------------------------
