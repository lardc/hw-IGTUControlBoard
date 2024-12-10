// Header
//
#include "Certification.h"

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
#include "TOCUHP.h"

// Definitions
//
#define SERT_PULSE_WIDTH_MS				10000	// ìêñ
#define SERT_VG_FRONT_TIME				5000	// ìêñ
//
#define SERT_AVG_START_INDEX_DEF		20
#define SERT_AVG_LENGTH					20
#define SERT_AVG_V_START_INDEX			950
#define SERT_AVG_VN_START_INDEX			400
#define SERT_AVG_I_START_INDEX			800
#define V_SOURCE_R_INT					34		// Îì

// Types
//
typedef enum __SertificationStage
{
	InitialPulse = 0,
	MainPulse = 1
} SertificationStage;

// Variables
//
LogParamsStruct SertificationLog;
MeasureSample SertificationSampledData;
RingBuffersParams SertificationRingBuffers;
SertificationStage Stage = InitialPulse;
float SetVoltageCorr = 0, SetVoltage;
//
pFloat32 SertSet_V, SertSet_Vneg, SertSet_I, SertSet_Tp, SertResult_V, SertResult_I, SertResult_Q;
RegulatorMode Mode;

// Functions prototypes
//
void SERT_V_CacheVariables();
void SERT_I_CacheVariables();
MeasureSample SERT_GetAverageSamples();

// Functions
//
void SERT_V_CacheVariables()
{
	CU_LoadConvertParams();
	REGULATOR_ResetVariables(&RegulatorParams);
	REGULATOR_CacheVariables(&RegulatorParams);
	REGULATOR_Mode(&RegulatorParams, Mode);
	LOG_ClearBuffers(&SertificationRingBuffers);

	SetVoltage = (Stage == InitialPulse) ? *SertSet_V : SetVoltageCorr;
	RegulatorParams.dVg = SetVoltage / (SERT_VG_FRONT_TIME / TIMER15_uS);
	RegulatorParams.Counter = SERT_PULSE_WIDTH_MS / TIMER15_uS;

	SertificationLog.DataA = &SertificationSampledData.Voltage;
	SertificationLog.DataB = &SertificationSampledData.Current;
	SertificationLog.LogBufferA = &CONTROL_VoltageValues[0];
	SertificationLog.LogBufferB = &CONTROL_CurrentValues[0];
	SertificationLog.LogBufferCounter = &CONTROL_Values_Counter;
	//
	SertificationRingBuffers.DataA = &SertificationSampledData.Voltage;
	SertificationRingBuffers.DataB = &SertificationSampledData.Current;
	SertificationRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
}
//------------------------------

void SERT_I_CacheVariables()
{
	CU_LoadConvertParams();
	//
	SertificationRingBuffers.RingCounterMask = LOG_COUNTER_MASK;
}
//------------------------------

void SERT_V_Prepare()
{
	Int16U CurrentRange = MEASURE_V_SetCurrentRange(*SertSet_I);

	INITCFG_ConfigADC_VgsIges(CurrentRange);
	INITCFG_ConfigDMA_VgsIges();
	MEASURE_ResetDMABuffers();
	LL_V_ShortOut(false);
	LL_V_ShortPAU(true);
	CONTROL_SwitchOutMUX(Voltage);

	SERT_V_CacheVariables();
	CONTROL_ResetOutputRegisters();

	CONTROL_SetDeviceState(CONTROL_State, SS_Sert_V_Process);
	CONTROL_StartHighPriorityProcesses();
}
//------------------------------

void SERT_I_Prepare()
{
	SERT_I_CacheVariables();

	CONTROL_SwitchOutMUX(Current);
	TOCUHP_PowerDisable();
	TOCUHP_EmulatedState(true);

	INITCFG_ConfigADC_Qg_I();
	INITCFG_ConfigDMA_Qg(ADC_DMA_BUFF_SIZE_QG);
	MEASURE_ResetDMABuffers();
	QG_SetPulseWidth(*SertSet_Tp);
	LL_ExDACVCutoff(CU_I_VcutoffToDAC(*SertSet_V));
	LL_ExDACVNegative(CU_I_VnegativeToDAC(*SertSet_Vneg));
	LL_I_SetDAC(CU_I_ItoDAC(*SertSet_I));
	LL_I_Start(false);
	LL_QgProtection(false);
	LL_I_Enable(true);
	DELAY_MS(20);

	CONTROL_ResetOutputRegisters();
	CONTROL_SetDeviceState(CONTROL_State, SS_Sert_I_Process);
	CONTROL_StartHighPriorityProcesses();
	CONTROL_HandleExternalLamp(true);

	LL_SyncOSC(true);
	DELAY_US(100);

	MEASURE_StartNewSampling();
	QG_Pulse(true);
}
//------------------------------

void SERT_V_Process()
{
	SertificationSampledData = MEASURE_V_SampleVI();

	LOG_SaveSampleToRingBuffer(&SertificationRingBuffers);
	LOG_LoggingData(&SertificationLog);

	if(RegulatorParams.Target < SetVoltage)
		RegulatorParams.Target += RegulatorParams.dVg;
	else
	{
		RegulatorParams.Target = SetVoltage;
		LL_SyncOSC(Stage == MainPulse);
	}

	RegulatorParams.SampledData = SertificationSampledData.Voltage;

	if(REGULATOR_Process(&RegulatorParams))
	{
		CONTROL_StopHighPriorityProcesses();

		if(RegulatorParams.FollowingError)
		{
			*SertResult_V = 0;
			*SertResult_I = 0;

			if(SertificationSampledData.Current > VGS_CURRENT_MAX)
			{
					DataTable[REG_PROBLEM] = PROBLEM_SHORT;
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
				CONTROL_SwitchToFault(DF_FOLLOWING_ERROR);
		}
		else
		{
			MeasureSample AverageData = LOG_RingBufferGetAverage(&SertificationRingBuffers);

			if(Stage == InitialPulse)
			{
				float Rload = SetVoltage / (AverageData.Current / 1000) - V_SOURCE_R_INT;
				float Iload = SetVoltage / Rload;
				SetVoltageCorr = SetVoltage + V_SOURCE_R_INT * Iload;
				Stage = MainPulse;
				CONTROL_SetDeviceState(DS_InProcess, SS_Sert_V_Prepare);
			}
			else
			{
				DataTable[REG_VGS_RESULT] = AverageData.Voltage;
				DataTable[REG_VGS_I_RESULT] = AverageData.Current;

				DataTable[REG_OP_RESULT] = OPRESULT_OK;

				Stage = InitialPulse;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
		}
	}
}
//-----------------------------------------------

void SERT_I_Process()
{
	if(DMA_IsTransferComplete(DMA1, DMA_ISR_TCIF1))
	{
		CONTROL_StopHighPriorityProcesses();

		LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], ADC_DMA_BUFF_SIZE_QG);
		CONTROL_Values_Counter = VALUES_x_SIZE;

		*SertResult_Q = QG_CalculateGateCharge(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
		*SertResult_I = LOG_GetAverageFromBuffer(&CONTROL_CurrentValues[SERT_AVG_I_START_INDEX], SERT_AVG_LENGTH);

		TOCUHP_EmulatedState(false);
		DataTable[REG_OP_RESULT] = OPRESULT_OK;
		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}
	else
	{
		if(DMA_ReadDataCount(DMA1_Channel1) <= ADC_DMA_BUFF_SIZE_QG / 2)
		{
			LL_QgProtection(true);
			LL_I_Start(true);
		}
	}
}
//-----------------------------------------------
