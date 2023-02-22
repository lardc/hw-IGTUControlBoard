// Header
//
#include "Vgs.h"

// Includes
//
#include "InitConfig.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Measurement.h"
#include "ConvertUtils.h"
#include "Regulator.h"
#include "Logging.h"

// Variables
//
LogParamsStruct VgsLog;
MeasureSample VgsSampledData;
RingBuffersParams VgsRingBuffers;
float TrigCurrentHigh = 0;
float TrigCurrentLow = 0;

// Function prototypes
//
void VGS_CacheVariables();


// Functions
//
void VGS_Prepare()
{
	INITCFG_ConfigADC_VgsIges();
	INITCFG_ConfigDMA_VgsIges();

	MEASURE_V_SetCurrentRange(DataTable[REG_VGS_I_TRIG]);

	LL_V_IlimHighRange();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	VGS_CacheVariables();

	CONTROL_SetDeviceState(DS_Ready, SS_Cal_V_Process);
	CONTROL_StartHighPriorityProcesses();
}
//-----------------------------

void VGS_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);

	RegulatorParams.dVg = DataTable[REG_VGS_FAST_RATE] * TIMER15_uS;
	RegulatorParams.Counter = DataTable[REG_VGS_V_MAX] / (DataTable[REG_VGS_FAST_RATE] + DataTable[REG_VGS_SLOW_RATE]) * TIMER15_uS;

	VgsLog.DataA = &VgsSampledData.Voltage;
	VgsLog.DataB = &VgsSampledData.Current;
	VgsLog.LogBufferA = &CONTROL_VoltageValues[0];
	VgsLog.LogBufferB = &CONTROL_CurrentValues[0];
	VgsLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	VgsRingBuffers.DataA = &VgsSampledData.Voltage;
	VgsRingBuffers.DataB = &VgsSampledData.Current;

	TrigCurrentHigh = DataTable[REG_VGS_I_TRIG];
	TrigCurrentLow = DataTable[REG_VGS_I_TRIG] - DataTable[REG_VGS_I_TRIG] * DataTable[REG_VGS_dI_TRIG] / 100;
}
//-----------------------------

void VGS_Process()
{
	VgsSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&VgsRingBuffers);
	LOG_LoggingData(&VgsLog);

	if(VgsSampledData.Current >= TrigCurrentLow)
		RegulatorParams.dVg = DataTable[REG_VGS_SLOW_RATE] * TIMER15_uS;

	if(VgsSampledData.Current < TrigCurrentHigh)
	{
		if(RegulatorParams.Target < DataTable[REG_VGS_V_MAX])
			RegulatorParams.Target += RegulatorParams.dVg;
		else
			RegulatorParams.Target = DataTable[REG_VGS_V_MAX];

		RegulatorParams.SampledData = VgsSampledData.Voltage;
	}
	else
	{
		CONTROL_StopHighPriorityProcesses();

		MeasureSample VgsResult = LOG_RingBufferGetAverage(&VgsRingBuffers);
		DataTable[REG_VGS_RESULT] = VgsResult.Voltage;
		DataTable[REG_VGS_I_RESULT] = VgsResult.Current;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;

		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_StopHighPriorityProcesses();

		DataTable[REG_VGS_RESULT] = 0;
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;

		if(RegulatorParams.FollowingError)
		{
			if(VgsSampledData.Current > TrigCurrentHigh)
			{
				DataTable[REG_PROBLEM] = PROBLEM_GATE_SHORT;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}

		if(VgsSampledData.Current < TrigCurrentHigh)
		{
			DataTable[REG_PROBLEM] = PROBLEM_VGS_CURRENT_NOT_REACHED;
			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------
