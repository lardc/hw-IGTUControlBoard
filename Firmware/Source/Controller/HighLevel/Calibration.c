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
#include "Delay.h"

// Definitions
//
#define CAL_PULSE_WIDTH_MS			10000	// ìêñ
#define CAL_VG_FRONT_TIME			5000	// ìêñ
//
#define CAL_AVG_START_INDEX			20
#define CAL_AVG_LENGTH				20

// Variables
//
LogParamsStruct CalibrationLog;
MeasureSample CalSampledData;
RingBuffersParams CalRingBuffers;

// Functions prototypes
//
void CAL_V_CacheVariables();
void CAL_I_CacheVariables();
MeasureSample CAL_GetAverageSamples();

// Functions
//
void CAL_V_CacheVariables()
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

void CAL_I_CacheVariables()
{
	CU_LoadConvertParams();
	MEASURE_ResetDMABuffers();

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

void CAL_V_Prepare()
{
	INITCFG_ConfigADC_VgsIges();
	INITCFG_ConfigDMA_VgsIges();

	MEASURE_V_SetCurrentRange(DataTable[REG_CAL_I]);
	LL_V_IlimHighRange();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	CAL_V_CacheVariables();

	CONTROL_SetDeviceState(DS_Ready, SS_Cal_V_Process);
	CONTROL_StartHighPriorityProcesses();
}
//------------------------------

void CAL_I_Prepare()
{
	INITCFG_ConfigADC_Qg();
	INITCFG_ConfigDMA_Qg();

	LL_ExDACVCutoff(CU_I_VcutoffToDAC(DataTable[REG_CAL_V]));
	LL_ExDACVNegative(CU_I_VnegativeToDAC(DataTable[REG_CAL_V]));
	LL_I_SetDAC(CU_I_ItoDAC(DataTable[REG_CAL_I]));
	LL_I_Start(false);
	LL_I_Enable(true);
	DELAY_MS(20);

	CONTROL_SwitchOutMUX(Voltage);

	CAL_I_CacheVariables();

	CONTROL_SetDeviceState(DS_Ready, SS_Cal_I_Process);
	MEASURE_StartNewSampling();
	CONTROL_StartHighPriorityProcesses();
}
//------------------------------

void CAL_V_CalProcess()
{
	CalSampledData = MEASURE_V_SampleVI();

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
			MeasureSample AverageData = LOG_RingBufferGetAverage(&CalRingBuffers);
			DataTable[REG_CAL_V_RESULT] = AverageData.Voltage;
			DataTable[REG_CAL_I_RESULT] = AverageData.Current;

			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------------------------

void CAL_I_CalProcess()
{
	if(DMA_IsTransferComplete(DMA1, DMA_ISR_TCIF1))
	{
		LL_I_Start(false);

		LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], sizeof(MEASURE_Qg_DataRaw), 1);
		LOG_CopyVoltageToEndpoints(&CONTROL_VoltageValues[0], &MEASURE_Qg_DataRaw[1], sizeof(MEASURE_Qg_DataRaw), 1);
		CONTROL_Values_Counter = VALUES_x_SIZE;

		DataTable[REG_CAL_V_RESULT] = LOG_GetAverageFromBuffer(&CONTROL_VoltageValues[CAL_AVG_START_INDEX], CAL_AVG_LENGTH);
		DataTable[REG_CAL_I_RESULT] = LOG_GetAverageFromBuffer(&CONTROL_CurrentValues[CAL_AVG_START_INDEX], CAL_AVG_LENGTH);

		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
		LL_I_Start(true);
}
//-----------------------------------------------
