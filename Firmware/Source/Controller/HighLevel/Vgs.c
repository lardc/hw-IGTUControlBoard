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

// Definitions
//
#define VGS_RING_BUFFER_SIZE				8
#define VGS_RING_BUFFER_CNT_MASK			VGS_RING_BUFFER_SIZE - 1

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
	Int16U CurrentRange = MEASURE_V_SetCurrentRange(DataTable[REG_VGS_I_TRIG]);

	INITCFG_ConfigADC_VgsIges(CurrentRange);
	INITCFG_ConfigDMA_VgsIges();
	MEASURE_ResetDMABuffers();
	LOG_ClearBuffers(&VgsRingBuffers);
	LL_V_IlimHighRange();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);
	REGULATOR_Mode(&RegulatorParams, FeedBack);

	VGS_CacheVariables();

	CONTROL_SetDeviceState(DS_InProcess, SS_VgsProcess);
	CONTROL_StartHighPriorityProcesses();

	LL_SyncOSC(true);
}
//-----------------------------

void VGS_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);

	RegulatorParams.dVg = DataTable[REG_VGS_FAST_RATE] * TIMER15_uS;
	RegulatorParams.Counter = DataTable[REG_VGS_V_MAX] / DataTable[REG_VGS_SLOW_RATE] * TIMER15_uS;

	VgsLog.DataA = &VgsSampledData.Voltage;
	VgsLog.DataB = &VgsSampledData.Current;
	VgsLog.LogBufferA = &CONTROL_VoltageValues[0];
	VgsLog.LogBufferB = &CONTROL_CurrentValues[0];
	VgsLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	VgsRingBuffers.DataA = &VgsSampledData.Voltage;
	VgsRingBuffers.DataB = &VgsSampledData.Current;
	VgsRingBuffers.RingCounterMask = VGS_RING_BUFFER_CNT_MASK;

	TrigCurrentHigh = DataTable[REG_VGS_I_TRIG];
	TrigCurrentLow = DataTable[REG_VGS_I_TRIG] - DataTable[REG_VGS_I_TRIG] * DataTable[REG_VGS_dI_TRIG] / 100;
}
//-----------------------------

void VGS_Process()
{
	MeasureSample AverageSamples;

	VgsSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&VgsRingBuffers);
	LOG_LoggingData(&VgsLog);

	AverageSamples = LOG_RingBufferGetAverage(&VgsRingBuffers);

	if(VgsSampledData.Current >= TrigCurrentLow)
		RegulatorParams.dVg = DataTable[REG_VGS_SLOW_RATE] * TIMER15_uS;

	if(VgsSampledData.Current < TrigCurrentHigh)
	{
		if(RegulatorParams.Target < DataTable[REG_VGS_V_MAX])
			RegulatorParams.Target += RegulatorParams.dVg;
		else
			RegulatorParams.Target = DataTable[REG_VGS_V_MAX];

		RegulatorParams.SampledData = VgsSampledData.Voltage;

		if(REGULATOR_Process(&RegulatorParams))
		{
			CONTROL_StopHighPriorityProcesses();
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;

			if(RegulatorParams.FollowingError)
			{
				DataTable[REG_PROBLEM] = PROBLEM_DUT_NOT_FOUND;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
				return;
			}
			else
			{
				if(VgsSampledData.Current < TrigCurrentLow)
				{
					DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
			}
		}
	}
	else
	{
		CONTROL_StopHighPriorityProcesses();

		if(VgsSampledData.Voltage < VGS_VOLTAGE_MIN)
		{
			DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
			DataTable[REG_PROBLEM] = PROBLEM_SHORT;
			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
		else
		{
			DataTable[REG_VGS_RESULT] = AverageSamples.Voltage;
			DataTable[REG_VGS_I_RESULT] = VgsSampledData.Current;
			DataTable[REG_OP_RESULT] = OPRESULT_OK;

			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
	}
}
//-----------------------------
