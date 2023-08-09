// Header
//
#include "Qg.h"

// Includes
//
#include "TOCUHP.h"
#include "Controller.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "InitConfig.h"
#include "LowLevel.h"
#include "ConvertUtils.h"
#include "Delay.h"
#include "Measurement.h"
#include "Logging.h"
#include "Global.h"
#include "stdlib.h"
#include "math.h"

// Definitions
//
#define QG_AVG_LENGTH					20
//
#define TOCUHP_WAIT_READY_TIMEOUT		10000
#define QG_FILTER_COEFFICIENT			0.1
#define QG_GATE_VOLTAGE_ALLOWED_ERR		10
#define QG_I_PULSE_WIDTH_COEF			0.96
#define QG_PULSE_WIDTH_THRE_COEF		0.95
#define QG_HW_CONFIG_TIME				5
#define QG_I_THRESHOLD_COEF				0.8

// Types
//
typedef enum __QgPrepareStage
{
	TOCUHP_InitWaiting = 0,
	TOCUHP_ConfigVoltage,
	TOCUHP_ConfigCurrent,
	TOCUHP_WatingReady,
	HW_Config,
	HW_ConfigWaiting,
	StartMeasure
} QgPrepareStage;

// Variables
//
QgPrepareStage QgConfigStage = TOCUHP_InitWaiting;
QgPrepareStage QgConfigLastStage = TOCUHP_InitWaiting;
Int64U TOCUHP_StateTimeout = 0;
Int64U Timeout = 0;

// Function prototypes
//
void QG_CacheVariables();
float QG_CalculateGateCharge(pFloat32 Buffer, Int16U BufferSize);
void QG_SetPulseWidth(float Period);
void QG_Filter(pFloat32 InputArray, Int16U ArraySize);
float QG_ExtractAverageCurrent(pFloat32 Buffer, Int16U BufferSize);
int QG_SortCondition(const void *A, const void *B);
void QG_CheckGateVoltage();

// Functions
//
void QG_Prepare()
{
	switch(QgConfigStage)
	{
		case TOCUHP_InitWaiting:
			TOCUHP_UpdateCANid();
			TOCUHP_StateTimeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
			QgConfigLastStage = TOCUHP_InitWaiting;
			QgConfigStage = TOCUHP_WatingReady;
			break;

		case TOCUHP_WatingReady:
			if(TOCUHP_IsReady())
			{
				switch(QgConfigLastStage)
				{
					case TOCUHP_InitWaiting:
						QgConfigStage = HW_Config;
						break;

					case TOCUHP_ConfigVoltage:
						QgConfigStage = TOCUHP_ConfigCurrent;
						break;

					case TOCUHP_ConfigCurrent:
						QgConfigStage = StartMeasure;
						break;

					default:
						break;
				}
			}
			else
			{
				if(CONTROL_TimeCounter >= TOCUHP_StateTimeout)
					CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
			}
			break;

		case HW_Config:
			CONTROL_SwitchOutMUX(Current);
			QG_CacheVariables();

			INITCFG_ConfigADC_Qg_I();
			INITCFG_ConfigDMA_Qg(VALUES_x_SIZE);
			MEASURE_ResetDMABuffers();
			QG_SetPulseWidth(DataTable[REG_QG_I_DURATION]);

			LL_ExDACVCutoff(CU_I_VcutoffToDAC(DataTable[REG_QG_V_CUTOFF]));
			LL_ExDACVNegative(CU_I_VnegativeToDAC(DataTable[REG_QG_V_NEGATIVE]));
			LL_I_SetDAC(CU_I_ItoDAC(DataTable[REG_QG_I]));
			LL_I_Enable(true);

			Timeout = CONTROL_TimeCounter + QG_HW_CONFIG_TIME;
			QgConfigStage = HW_ConfigWaiting;
			break;

		case HW_ConfigWaiting:
			if(CONTROL_TimeCounter >= Timeout)
				QgConfigStage = TOCUHP_ConfigVoltage;
			break;

		case TOCUHP_ConfigVoltage:
			if(TOCUHP_ConfigAnodeVoltage(DataTable[REG_QG_V_POWER]))
			{
				TOCUHP_StateTimeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
				QgConfigLastStage = TOCUHP_ConfigVoltage;
				QgConfigStage = TOCUHP_WatingReady;
			}
			break;

		case TOCUHP_ConfigCurrent:
			if(TOCUHP_ConfigAnodeCurrent(DataTable[REG_QG_I_POWER]))
			{
				TOCUHP_StateTimeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
				QgConfigLastStage = TOCUHP_ConfigCurrent;
				QgConfigStage = TOCUHP_WatingReady;
			}
			break;

		case StartMeasure:
			CONTROL_SetDeviceState(CONTROL_State, SS_QgProcess);
			MEASURE_StartNewSampling();
			CONTROL_HandleExternalLamp(true);

			LL_QgProtection(true);
			DELAY_US(10);

			LL_SyncTOCUHP(true);
			QG_Pulse(true);
			break;
	}
}
//----------------------------

void QG_CacheVariables()
{
	CU_LoadConvertParams();
}
//----------------------------

void QG_SetPulseWidth(float Period)
{
	INITCFG_ConfigTimer3(Period * QG_I_PULSE_WIDTH_COEF);
}
//----------------------------

void QG_Pulse(bool State)
{
	LL_SyncOSC(State);

	(State) ? TIM_Start(TIM3) : TIM_Stop(TIM3);

	TIM_Reset(TIM3);
	LL_I_Start(State);
}
//----------------------------

void QG_CheckGateVoltage()
{
	float CurrentWidth = 0;

	for(int i = 0; i < VALUES_x_SIZE; i++)
	{
		if(CONTROL_CurrentValues[i] > 0)
			CurrentWidth += QG_CURRENT_SAMPLE_TIME;
	}

	if(CurrentWidth > DataTable[REG_QG_I_DURATION] * QG_PULSE_WIDTH_THRE_COEF)
		DataTable[REG_WARNING] = WARNING_QG_VOLTAGE_NOT_REACHED;
}
//----------------------------

void QG_SaveResult()
{
	float Ig, Qg;

	if(TOCUHP_InFault())
		CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
	else
	{
		if(TOCUHP_CheckOpResult() || CONTROL_State == DS_SelfTest)
		{
			LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], ADC_DMA_BUFF_SIZE_QG / 2, 0);
			CONTROL_Values_Counter = VALUES_x_SIZE;
			QG_Filter(&CONTROL_CurrentValues[0], VALUES_x_SIZE);

			Ig = QG_ExtractAverageCurrent(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
			Qg = QG_CalculateGateCharge(&CONTROL_CurrentValues[0], VALUES_x_SIZE);

			if(Ig > DataTable[REG_QG_I] * QG_I_THRESHOLD_COEF)
			{
				if(Qg < MEASURE_QG_MIN || Qg > MEASURE_QG_MAX)
					DataTable[REG_WARNING] = WARNING_OUT_OF_RANGE;

				DataTable[REG_QG_I_RESULT] =  Ig;
				DataTable[REG_QG_RESULT] = Qg;

				QG_CheckGateVoltage();
				DataTable[REG_OP_RESULT] = OPRESULT_OK;
			}
			else
			{
				DataTable[REG_QG_I_RESULT] =  0;
				DataTable[REG_QG_RESULT] = 0;
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
			}

			QG_ResetConfigStageToDefault();
			CONTROL_ResetHardwareToDefaultState();

			CONTROL_SetDeviceState(DS_Ready, SS_None);
		}
		else
			CONTROL_SwitchToFault(DF_POWER_CURRENT);
	}
}
//-----------------------------------------------

void QG_ResetConfigStageToDefault()
{
	QgConfigStage = TOCUHP_InitWaiting;
}
//-----------------------------------------------

float QG_CalculateGateCharge(pFloat32 Buffer, Int16U BufferSize)
{
	float GateCharge = 0;
	float Current = 0;

	for(int i = 0; i < BufferSize; i++)
	{
		Current = *(Buffer + i);

		if(Current > 0)
			GateCharge += *(Buffer + i) * QG_CURRENT_SAMPLE_TIME;
	}

	return GateCharge;
}
//-----------------------------------------------

void QG_Filter(pFloat32 InputArray, Int16U ArraySize)
{
	float FilteredData = 0;

	for(int i = 0; i < ArraySize; i++)
	{
		FilteredData += (*(InputArray + i) - FilteredData) * QG_FILTER_COEFFICIENT;
		  *(InputArray + i) = FilteredData;
	}
}
//-----------------------------------------------

float QG_ExtractAverageCurrent(pFloat32 Buffer, Int16U BufferSize)
{
	float AverageValue = 0;
	static float CopiedBuffer[VALUES_x_SIZE];

	for (int i = 0; i < BufferSize; i++)
		CopiedBuffer[i] = *(Buffer + i);

	qsort(CopiedBuffer, BufferSize, sizeof(*CopiedBuffer), QG_SortCondition);

	for (int i = BufferSize; i >= BufferSize - QG_AVG_LENGTH; i--)
		AverageValue += *(CopiedBuffer + i);

	return (AverageValue / QG_AVG_LENGTH);
}
//-----------------------------------------------

int QG_SortCondition(const void *A, const void *B)
{
	return (float)(*(float *)A) - (float)(*(float *)B);
}
//-----------------------------------------
