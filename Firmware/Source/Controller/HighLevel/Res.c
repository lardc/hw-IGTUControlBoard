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
#include "ConvertUtils.h"
#include "Global.h"
#include "Logging.h"
#include "Delay.h"

// Definitions
//
#define RES_PULSE_WIDTH_MS			10000	// ìêñ
#define RES_VG_FRONT_TIME			5000	// ìêñ
#define RES_TEST_VOLTAGE			10.0f	// Â
#define LINE_SHORT_CURRENT			5
#define LINE_RESISTANCE				4		// Îì
//
#define RES_AVG_START_INDEX_DEF		20
#define RES_AVG_LENGTH				20
#define RES_AVG_V_START_INDEX		800
#define RES_AVG_VN_START_INDEX		400
#define RES_AVG_I_START_INDEX		800

// Variables
//
LogParamsStruct ResMeasureLog;
MeasureSample ResSampledData;
RingBuffersParams ResRingBuffers;
bool ResFineMeasure = false;
float ResTestCurrent = V_I_R2_MAX;

// Functions prototypes
//
void RES_CacheVariables();
MeasureSample RES_GetAverageSamples();

// Functions
//
void RES_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);
	REGULATOR_Mode(&RegulatorParams, FeedBack);
	LOG_ClearBuffers(&ResRingBuffers);

	RegulatorParams.dVg = RES_TEST_VOLTAGE / (RES_VG_FRONT_TIME / TIMER15_uS);
	RegulatorParams.Counter = RES_PULSE_WIDTH_MS / TIMER15_uS;

	ResMeasureLog.DataA = &ResSampledData.Voltage;
	ResMeasureLog.DataB = &ResSampledData.Current;
	ResMeasureLog.LogBufferA = &CONTROL_VoltageValues[0];
	ResMeasureLog.LogBufferB = &CONTROL_CurrentValues[0];
	ResMeasureLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	ResRingBuffers.DataA = &ResSampledData.Voltage;
	ResRingBuffers.DataB = &ResSampledData.Current;
	ResRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
}
//------------------------------

void RES_Prepare()
{
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

void RES_Process()
{
	ResSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&ResRingBuffers);
	LOG_LoggingData(&ResMeasureLog);

	if(RegulatorParams.Target < RES_TEST_VOLTAGE)
		RegulatorParams.Target += RegulatorParams.dVg;
	else
	{
		RegulatorParams.Target = RES_TEST_VOLTAGE;
		LL_SyncOSC(true);
		IsImpulse = true;
	}

	RegulatorParams.SampledData = ResSampledData.Voltage;

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_StopHighPriorityProcesses();

		if(RegulatorParams.FollowingError)
		{
			DataTable[REG_RES_RESULT] = 0;
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;

			if(ResSampledData.Current > LINE_SHORT_CURRENT)
			{
				DataTable[REG_PROBLEM] = PROBLEM_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}
		else
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
				ResTestCurrent = V_I_R2_MAX;

				DataTable[REG_RES_RESULT] = (AverageData.Voltage - LINE_RESISTANCE * AverageData.Current / 1000) / AverageData.Current * 1000;

				if(DataTable[REG_RES_RESULT] < MEASURE_RES_MIN || DataTable[REG_RES_RESULT] > MEASURE_RES_MAX)
					DataTable[REG_WARNING] = WARNING_OUT_OF_RANGE;

				DataTable[REG_OP_RESULT] = OPRESULT_OK;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
		}
	}
}
//-----------------------------------------------
