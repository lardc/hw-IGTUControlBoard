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

// Variables
//
LogParamsStruct IgesLog;
MeasureSample IgesSampledData;
RingBuffersParams IgesRingBuffers;

// Function prototypes
//
void IGES_CacheVariables();

// Functions
//
void IGES_Prepare()
{
	if(PAU_Configure(PAU_CHANNEL_IGTU, PAU_AUTO_RANGE, DataTable[REG_IGES_PLATE_TIME]))
	{
		INITCFG_ConfigADC_VgsIges();
		INITCFG_ConfigDMA_VgsIges();

		MEASURE_V_SetCurrentRange(VGS_CURRENT_MAX);

		LL_V_IlimHighRange();
		LL_V_ShortOut(false);
		LL_V_ShortPAU(true);
		CONTROL_SwitchOutMUX(Voltage);

		IGES_CacheVariables();

		CONTROL_SetDeviceState(DS_Ready, SS_IgesProcess);
		CONTROL_StartHighPriorityProcesses();
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		CONTROL_SwitchToFault(DF_PAU_INTERFACE);
	}
}
//-----------------------------------------------

void IGES_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);

	RegulatorParams.dVg = DataTable[REG_VGS_FAST_RATE] * TIMER15_uS;
	RegulatorParams.Counter = DataTable[REG_IGES_V] / DataTable[REG_IGES_V_RATE] * TIMER15_uS + DataTable[REG_IGES_PLATE_TIME] * 1000 / TIMER15_uS;

	IgesLog.DataA = &IgesSampledData.Voltage;
	IgesLog.DataB = &IgesSampledData.Current;
	IgesLog.LogBufferA = &CONTROL_VoltageValues[0];
	IgesLog.LogBufferB = &CONTROL_CurrentValues[0];
	IgesLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	IgesRingBuffers.DataA = &IgesSampledData.Voltage;
	IgesRingBuffers.DataB = &IgesSampledData.Current;
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
			IgesRingBuffers.DataB = NULL;
			IgesLog.DataB = NULL;
			IgesLog.LogBufferB = NULL;
			RegulatorParams.Target = DataTable[REG_IGES_V];
			LL_V_IlimLowRange();
		}
	}

	RegulatorParams.SampledData = IgesSampledData.Voltage;

	if(REGULATOR_Process(&RegulatorParams))
	{
		if(RegulatorParams.FollowingError)
		{
			CONTROL_StopHighPriorityProcesses();

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

		CONTROL_SetDeviceState(DS_InProcess, SS_IgesSaveResult);
	}
}
//-----------------------------------------------

void IGES_SaveResults()
{
	float Iges = 0;

	if(PAU_ReadMeasuredData(&Iges))
	{
		if(Iges < MEASURE_IGES_CURRENT_MIN)
			DataTable[REG_WARNING] = WARNING_IGES_TOO_LOW;
		if(Iges > MEASURE_IGES_CURRENT_MAX)
			DataTable[REG_WARNING] = WARNING_IGES_TOO_HIGH;

		DataTable[REG_IGES_RESULT] = Iges;
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
	}
	else
	{
		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		CONTROL_SwitchToFault(DF_PAU_INTERFACE);
	}

	CONTROL_StopHighPriorityProcesses();
	CONTROL_SetDeviceState(DS_Ready, SS_None);
}
//-----------------------------------------------
