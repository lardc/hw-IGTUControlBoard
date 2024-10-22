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
#include "Math.h"

// Definitions
//
#define PAU_SYNC_DELAY_US				500
#define PAU_SYNC_DELAY_STEP				(Int16U)(PAU_SYNC_DELAY_US / TIMER15_uS)
//
#define PAU_UDATE_STATE_PERIOD			50	// in ms
#define SHORT_CIRCUIT_CURRENT_ERROR		10	// %
#define DUT_NOT_FOUND_LEVEL				V_I_R0_MAX * 0.5
//
#define IGES_RING_BUFFER_SIZE			8
#define IGES_RING_BUFFER_CNT_MASK		IGES_RING_BUFFER_SIZE - 1

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
float SampledCurrent;
IgesPrepareStage IgesConfigStage = PAU_Config;
Int64U PAU_StateTimeout = 0;
bool PAU_SyncFlag = false;
Int16U IgesSamplesCounter = 0;

// Function prototypes
//
void IGES_CacheVariables();
void IGES_PAUsyncProcess(bool State);
bool IGES_CheckDUT(bool PulsePlate, float SampledCurrent);

// Functions
//
void IGES_Prepare()
{
	float PAU_Current = 0;

	if(DataTable[REG_PAU_EMULATED])
		IgesConfigStage = HW_Config;

	switch(IgesConfigStage)
	{
		case PAU_Config:
			if(PAU_IsReady() || PAU_IsConfigReady())
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
					IgesConfigStage = PAU_Wating;
				}
				else
					CONTROL_SwitchToFault(DF_PAU_INTERFACE);
			}
			else
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			break;

		case PAU_Wating:
			if(PAU_IsConfigReady())
				IgesConfigStage = HW_Config;
			else
			{
				if(CONTROL_TimeCounter >= PAU_StateTimeout)
					CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
			}
			break;

		case HW_Config:
			IgesConfigStage = PAU_Config;

			IGES_CacheVariables();
			Int16U CurrentRange = MEASURE_V_SetCurrentRange(V_I_R1_MAX);

			INITCFG_ConfigADC_VgsIges(CurrentRange);
			INITCFG_ConfigDMA_VgsIges();
			MEASURE_ResetDMABuffers();

			LL_V_ShortOut(false);
			PAU_ShortInput(true);
			CONTROL_SwitchOutMUX(Voltage);
			REGULATOR_Mode(&RegulatorParams, Parametric);

			CONTROL_SetDeviceState(DS_InProcess, SS_IgesProcess);
			CONTROL_StartHighPriorityProcesses();

			LL_SyncOSC(true);
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

	IgesLog.DataA = &RegulatorParams.Target;
	IgesLog.DataB = &SampledCurrent;
	IgesLog.LogBufferA = &CONTROL_VoltageValues[0];
	IgesLog.LogBufferB = &CONTROL_CurrentValues[0];
	IgesLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	IgesSamplesCounter = DataTable[REG_IGES_SAMPLES_NUMBER];
}
//-----------------------------------------------

void IGES_Process()
{
	static bool StartPulsePlate = false;

	SampledCurrent = MEASURE_V_SampleVI().Current;

	LOG_LoggingData(&IgesLog);

	if(RegulatorParams.Target < DataTable[REG_IGES_V])
	{
		RegulatorParams.Target += RegulatorParams.dVg;
		StartPulsePlate = false;
	}
	else
	{
		RegulatorParams.Target = DataTable[REG_IGES_V];
		StartPulsePlate = true;
	}

	if(IGES_CheckDUT(StartPulsePlate, SampledCurrent))
	{
		IGES_PAUsyncProcess(StartPulsePlate);

		REGULATOR_Process(&RegulatorParams);

		if(StartPulsePlate)
		{
			if(!IgesSamplesCounter)
			{
				StartPulsePlate = false;
				CONTROL_StopHighPriorityProcesses();

				PAU_StateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
				CONTROL_SetDeviceState(DS_InProcess, SS_IgesSaveResult);
			}
		}
	}
	else
	{
		StartPulsePlate = false;
		CONTROL_StopHighPriorityProcesses();
	}
}
//-----------------------------------------------

bool IGES_CheckDUT(bool PulsePlate, float SampledCurrent)
{
	static bool IsDUTChecked = false;
	static float CurrentMax = 0;

	if(PulsePlate)
	{
		if(!IsDUTChecked)
		{
			IsDUTChecked = true;

			float Error1 = fabsf((CurrentMax - V_I_R0_MAX) / V_I_R0_MAX * 100);
			float Error2 = fabsf((SampledCurrent - V_I_R0_MAX) / V_I_R0_MAX * 100);

			if(Error1 < SHORT_CIRCUIT_CURRENT_ERROR && Error2 < SHORT_CIRCUIT_CURRENT_ERROR)
			{
				CurrentMax = 0;
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				DataTable[REG_PROBLEM] = PROBLEM_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
				return false;
			}
			else
			{
				if(CurrentMax < DUT_NOT_FOUND_LEVEL && DataTable[REG_DUT_CONN_CHECK])
				{
					CurrentMax = 0;
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					DataTable[REG_PROBLEM] = PROBLEM_DUT_NOT_FOUND;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
					return false;
				}
			}

			CurrentMax = 0;
			PAU_ShortInput(false);
		}
	}
	else
	{
		IsDUTChecked = false;

		if(SampledCurrent > CurrentMax)
			CurrentMax = SampledCurrent;
	}

	return true;
}
//-----------------------------------------------

void IGES_PAUsyncProcess(bool State)
{
	static Int16U SyncDelayCounter = 0;
	static bool SyncState = false;
	static Int16U IgesSamplesLast = 0;
	static Int64U IgesSamplesTimeoutCounter = 0;

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

		if(IgesSamplesLast != IgesSamplesCounter)
		{
			IgesSamplesLast = IgesSamplesCounter;
			IgesSamplesTimeoutCounter = CONTROL_TimeCounter + PAU_SYNC_PERIOD_MAX;
		}
		else
		{
			if(CONTROL_TimeCounter > IgesSamplesTimeoutCounter)
			{
				CONTROL_StopHighPriorityProcesses();

				DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_PAU_SYNC_TIMEOUT);
			}
		}
	}
	else
	{
		IgesSamplesLast = 0;
		SyncDelayCounter = PAU_SYNC_DELAY_STEP;
		SyncState = false;
	}

	LL_SyncPAU(SyncState);
}
//-----------------------------------------------

void IGES_SaveResults()
{
	float Iges = 0;

	if(DataTable[REG_PAU_EMULATED])
	{
		DataTable[REG_IGES_RESULT] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;

		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
	{
		if(PAU_IsReady())
		{
			if(PAU_ReadMeasuredData(&Iges))
			{
				Iges = Iges * 1e6 - DataTable[REG_IGES_V] / DataTable[REG_CTRL_LINE_RES] * 1000; // in nA

				Iges = (Iges < 0) ? 0 : Iges;

				DataTable[REG_IGES_RESULT] = Iges;
				DataTable[REG_OP_RESULT] = OPRESULT_OK;

				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_PAU_INTERFACE);
		}
		else
		{
			if(CONTROL_TimeCounter >= PAU_StateTimeout)
				CONTROL_SwitchToFault(DF_PAU_WRONG_STATE);
		}
	}
}
//-----------------------------------------------
