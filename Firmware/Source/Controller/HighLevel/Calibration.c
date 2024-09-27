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
#include "Qg.h"

// Definitions
//
#define CAL_PULSE_WIDTH_MS			10000	// ìêñ
#define CAL_VG_FRONT_TIME			5000	// ìêñ
//
#define CAL_AVG_START_INDEX_DEF		20
#define CAL_V_AVG_LENGTH			20
#define CAL_I_AVG_LENGTH			75
#define CAL_AVG_V_START_INDEX		950
#define CAL_AVG_VN_START_INDEX		400
#define CAL_ADC_DMA_BUFF_SIZE		1000

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
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);
	REGULATOR_Mode(&RegulatorParams, Parametric);
	LOG_ClearBuffers(&CalRingBuffers);

	RegulatorParams.dVg = DataTable[REG_CAL_VP] / (CAL_VG_FRONT_TIME / TIMER15_uS);
	RegulatorParams.Counter = CAL_PULSE_WIDTH_MS / TIMER15_uS;

	CalibrationLog.DataA = &CalSampledData.Voltage;
	CalibrationLog.DataB = &CalSampledData.Current;
	CalibrationLog.LogBufferA = &CONTROL_VoltageValues[0];
	CalibrationLog.LogBufferB = &CONTROL_CurrentValues[0];
	CalibrationLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	CalRingBuffers.DataA = &CalSampledData.Voltage;
	CalRingBuffers.DataB = &CalSampledData.Current;
	CalRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
}
//------------------------------

void CAL_I_CacheVariables()
{
	CU_LoadConvertParams();
	//
	CalRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
}
//------------------------------

void CAL_V_Prepare()
{
	Int16U CurrentRange = MEASURE_V_SetCurrentRange(DataTable[REG_CAL_I]);

	INITCFG_ConfigADC_VgsIges(CurrentRange);
	INITCFG_ConfigDMA_VgsIges();
	MEASURE_ResetDMABuffers();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	CAL_V_CacheVariables();
	CONTROL_ResetOutputRegisters();

	CONTROL_SetDeviceState(CONTROL_State, SS_Cal_V_Process);
	CONTROL_StartHighPriorityProcesses();
}
//------------------------------

void CAL_I_Prepare()
{
	CONTROL_SwitchOutMUX(Current);
	CAL_I_CacheVariables();

	INITCFG_ConfigADC_Qg_I();
	INITCFG_ConfigDMA_Qg(CAL_ADC_DMA_BUFF_SIZE);
	MEASURE_ResetDMABuffers();
	LL_ExDACVCutoff(CU_I_VcutoffToDAC(DataTable[REG_CAL_VP]));
	LL_ExDACVNegative(CU_I_VnegativeToDAC(DataTable[REG_CAL_VN]));
	LL_I_SetDAC(CU_I_ItoDAC(DataTable[REG_CAL_I]));
	LL_I_Start(false);
	LL_QgProtection(false);
	LL_I_Enable(true);
	DELAY_MS(150);

	CONTROL_ResetOutputRegisters();
	CONTROL_SetDeviceState(CONTROL_State, SS_Cal_I_Process);
	MEASURE_StartNewSampling();
	CONTROL_StartHighPriorityProcesses();

	LL_SyncOSC(true);
	DELAY_US(20);
}
//------------------------------

void CAL_V_CalProcess()
{
	CalSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&CalRingBuffers);
	LOG_LoggingData(&CalibrationLog);

	if(RegulatorParams.Target < DataTable[REG_CAL_VP])
		RegulatorParams.Target += RegulatorParams.dVg;
	else
	{
		RegulatorParams.Target = DataTable[REG_CAL_VP];
		LL_SyncOSC(true);
	}

	RegulatorParams.SampledData = CalSampledData.Voltage;

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_StopHighPriorityProcesses();

		if(RegulatorParams.FollowingError)
		{
			DataTable[REG_CAL_V_RESULT] = 0;
			DataTable[REG_CAL_I_RESULT] = 0;

			if(CalSampledData.Current > VGS_CURRENT_MAX)
			{
				if(CONTROL_State == DS_SelfTest)
				{
					DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
				}
				else
				{
					DataTable[REG_PROBLEM] = PROBLEM_SHORT;
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}
		else
		{
			MeasureSample AverageData = LOG_RingBufferGetAverage(&CalRingBuffers);

			DataTable[REG_CAL_V_RESULT] = AverageData.Voltage;
			DataTable[REG_CAL_I_RESULT] = AverageData.Current;

			DataTable[REG_OP_RESULT] = OPRESULT_OK;

			if(CONTROL_State == DS_SelfTest)
				CONTROL_SetDeviceState(CONTROL_State, SS_V_Check);
			else
				CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------------------------

void CAL_I_CalProcess()
{
	if(DMA_IsTransferComplete(DMA1, DMA_ISR_TCIF1))
	{
		CONTROL_StopHighPriorityProcesses();

		LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], CAL_ADC_DMA_BUFF_SIZE);
		CONTROL_Values_Counter = VALUES_x_SIZE;
		QG_RemoveDC(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
		QG_Filter(CONTROL_CurrentValues, VALUES_x_SIZE);

		DataTable[REG_CAL_I_RESULT] = QG_ExtractAverageCurrent(&CONTROL_CurrentValues[0], VALUES_x_SIZE, CAL_I_AVG_LENGTH);

		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
	{
		if(DMA_ReadDataCount(DMA1_Channel1) <= CAL_ADC_DMA_BUFF_SIZE / 2)
		{
			LL_QgProtection(true);
			LL_I_Start(true);
		}
	}
}
//-----------------------------------------------
