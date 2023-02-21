// Header
//
#include "Calibration.h"

// Includes
//
#include "InitConfig.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "ConvertUtils.h"
#include "Global.h"
#include "Logging.h"

// Definitions
//
#define CAL_PULSE_WIDTH_MS			10000	// ìêñ
#define CAL_VG_FRONT_TIME			5000	// ìêñ

// Variables
//
LogParamsStruct CalibrationLog;
MeasureSample CalSampledData;
RingBuffersParams CalRingBuffers;

// Functions prototypes
//
void CAL_CacheVariables();
void CAL_V_CalProcess();
MeasureSample CAL_GetAverageSamples();

// Functions
//
void CAL_CacheVariables()
{
	CU_LoadConvertParams();
	MEASURE_ResetDMABuffers();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);

	RegulatorParams.dVg = DataTable[REG_CAL_V] / (CAL_VG_FRONT_TIME / TIMER15_uS);
	RegulatorParams.Counter = CAL_PULSE_WIDTH_MS / TIMER15_uS;

	CalibrationLog.DataA = &CalSampledData.Voltage;
	CalibrationLog.DataB = &CalSampledData.Current;
	CalibrationLog.LogBufferA = &CONTROL_VoltageValues[0];
	CalibrationLog.LogBufferB = &CONTROL_CurrentValues[0];
	CalibrationLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	CalRingBuffers.DataA = &CalSampledData.Voltage;
	CalRingBuffers.DataB = &CalSampledData.Current;
}
//------------------------------

void CAL_Prepare()
{
	INITCFG_ConfigADC_VgsIges();
	INITCFG_ConfigDMA_VgsIges();

	MEASURE_V_SetCurrentRange(DataTable[REG_CAL_I]);
	LL_V_CLimitHighRange();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	CAL_CacheVariables();
}
//------------------------------

void CAL_Calibration(DeviceSubState SubState)
{
	CalSampledData = MEASURE_SampleVgsIges();

	switch(SubState)
	{
	case SS_Cal_V_Process:
		CAL_V_CalProcess();
		break;

	default:
		break;
	}
}
//------------------------------

void CAL_V_CalProcess()
{
	LOG_SaveSampleToRingBuffer(&CalRingBuffers);
	LOG_LoggingData(&CalibrationLog);

	if(RegulatorParams.Target < DataTable[REG_CAL_V])
		RegulatorParams.Target += RegulatorParams.dVg;
	else
		RegulatorParams.Target = DataTable[REG_CAL_V];

	RegulatorParams.SampledData = CalSampledData.Voltage;

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_V_Stop();

		if(RegulatorParams.FollowingError)
		{
			DataTable[REG_CAL_V_RESULT] = 0;
			DataTable[REG_CAL_I_RESULT] = 0;

			if(CalSampledData.Current > MEASURE_CURRENT_MAX)
			{
				DataTable[REG_PROBLEM] = PROBLEM_GATE_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}
		else
		{
			MeasureSample AverageData = LOG_GetAverage(&CalRingBuffers);
			DataTable[REG_CAL_V_RESULT] = AverageData.Voltage;
			DataTable[REG_CAL_I_RESULT] = AverageData.Current;

			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------------------------
