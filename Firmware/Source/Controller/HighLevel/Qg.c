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
#define QG_FILTER_COEFFICIENT			0.2
#define QG_AVG_LENGTH					15
#define QG_AVG_IGNORE_POINTS			50
#define QG_DC_PART_STOP_INDEX			100
//
#define TOCUHP_WAIT_READY_TIMEOUT		10000
#define QG_HW_WAIT_TIME					100
#define QG_I_PULSE_WIDTH_COEF			0.96
#define QG_I_THRESHOLD_COEF				0.5
#define QG_CALC_NOISE_CUT_OFF_COEF		0.3

// Types
//
typedef enum __QgPrepareStage
{
	HW_Config = 0,
	HW_ConfigWaiting,
	TOCUHP_InitWaiting,
	TOCUHP_ConfigVoltage,
	TOCUHP_ConfigCurrent,
	TOCUHP_WatingReady,
	StartMeasure,
	GateDischarge
} QgPrepareStage;

// Variables
//
float QgVoltageCutOff, QgVoltageNegative, QgGateCurrent, QgGateCurrentDuration;
//
QgPrepareStage QgConfigStage = HW_Config;
QgPrepareStage QgConfigLastStage = HW_Config;
QGStages QGMeasureStage = QG_PreMeasurement;
Int64U Timeout = 0;
float QgCopiedBuffer[VALUES_x_SIZE];

// Function prototypes
//
int QG_SortCondition(const void *A, const void *B);
float QG_ExtractPulseWidth(pFloat32 Buffer, Int16U BufferSize);
void QG_CalculateData(pFloat32 PulseWidth, pFloat32 Ig, pFloat32 Qg);

// Functions
//
void QG_Prepare()
{
	switch(QgConfigStage)
	{
		case HW_Config:
			if(DataTable[REG_SERTIFICATION])
			{
				TOCUHP_PowerDisable();
				TOCUHP_EmulatedState(true);
				QGMeasureStage = QG_Sertification;
			}

			CONTROL_SwitchOutMUX(Current);

			INITCFG_ConfigADC_Qg_I();
			INITCFG_ConfigDMA_Qg(VALUES_x_SIZE);
			MEASURE_ResetDMABuffers();
			QG_SetPulseWidth(QgGateCurrentDuration);

			LL_ExDACVCutoff(CU_I_VcutoffToDAC(QgVoltageCutOff));
			LL_ExDACVNegative(CU_I_VnegativeToDAC(QgVoltageNegative));
			LL_I_SetDAC(CU_I_ItoDAC(QgGateCurrent));
			LL_I_Enable(true);

			Timeout = CONTROL_TimeCounter + QG_HW_WAIT_TIME;

			QgConfigStage = HW_ConfigWaiting;
			break;

		case HW_ConfigWaiting:
			if(CONTROL_TimeCounter >= Timeout)
				QgConfigStage = (QGMeasureStage == QG_MainMeasurement) ? StartMeasure : TOCUHP_InitWaiting;
			break;

		case TOCUHP_InitWaiting:
			TOCUHP_UpdateCANid();
			Timeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
			QgConfigLastStage = TOCUHP_InitWaiting;
			QgConfigStage = TOCUHP_WatingReady;
			break;

		case TOCUHP_WatingReady:
			if(TOCUHP_IsReady())
			{
				switch(QgConfigLastStage)
				{
					case TOCUHP_InitWaiting:
						QgConfigStage = TOCUHP_ConfigVoltage;
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
				if(CONTROL_TimeCounter >= Timeout)
					CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
			}
			break;

			case TOCUHP_ConfigVoltage:
				if(TOCUHP_ConfigAnodeVoltage(DataTable[REG_QG_V_POWER]))
				{
					Timeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
					QgConfigLastStage = TOCUHP_ConfigVoltage;
					QgConfigStage = TOCUHP_WatingReady;
				}
				break;

			case TOCUHP_ConfigCurrent:
				if(TOCUHP_ConfigAnodeCurrent(DataTable[REG_QG_I_POWER]))
				{
					Timeout = CONTROL_TimeCounter + TOCUHP_WAIT_READY_TIMEOUT;
					QgConfigLastStage = TOCUHP_ConfigCurrent;
					QgConfigStage = TOCUHP_WatingReady;
				}
				break;

		case StartMeasure:
			CONTROL_SetDeviceState(CONTROL_State, SS_QgProcess);
			CONTROL_HandleExternalLamp(true);

			LL_QgProtection(true);
			DELAY_US(10);
			LL_SyncTOCUHP(QGMeasureStage == QG_MainMeasurement);
			DELAY_US(500);
			LL_QgProtection(false);
			DELAY_US(2000);

			MEASURE_StartNewSampling();

			LL_QgProtection(true);
			QG_Pulse(true);
			DELAY_US(20);
			break;

		case GateDischarge:
			if(CONTROL_TimeCounter >= Timeout)
				QgConfigStage = HW_Config;
			break;
	}
}
//----------------------------

void QG_CacheVariables()
{
	QgVoltageCutOff = DataTable[REG_QG_V_CUTOFF];
	QgVoltageNegative = DataTable[REG_QG_V_NEGATIVE];
	QgGateCurrent = DataTable[REG_QG_I];
	QgGateCurrentDuration = DataTable[REG_QG_I_DURATION];

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

	if(State)
	{
		DELAY_US(50);
		TIM_Start(TIM3);
	}
	else
		TIM_Stop(TIM3);

	TIM_Reset(TIM3);
	LL_I_Start(State);
}
//----------------------------

void QG_SaveResult()
{
	float Ig, Qg, PulseWidth;

	DataTable[REG_SERTIFICATION] = 0;

	switch(QGMeasureStage)
	{
		case QG_PreMeasurement:
			QG_CalculateData(&PulseWidth, &Ig, &Qg);
			QgGateCurrent += QgGateCurrent - Ig;

			DEVPROFILE_ResetScopes(0);
			DEVPROFILE_ResetEPReadState();
			CONTROL_ResetHardwareToDefaultState();

			Timeout = CONTROL_TimeCounter + QG_HW_WAIT_TIME;
			QGMeasureStage = QG_MainMeasurement;
			QgConfigStage = GateDischarge;
			CONTROL_SetDeviceState(DS_InProcess, SS_QgPrepare);
		break;

		case QG_MainMeasurement:
			QGMeasureStage = QG_PreMeasurement;

			if(TOCUHP_InFault())
				CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
			else
			{
				if(TOCUHP_CheckOpResult() || CONTROL_State == DS_SelfTest)
				{
					QG_CalculateData(&PulseWidth, &Ig, &Qg);

					if(PulseWidth >= DataTable[REG_QG_I_DURATION] * QG_I_PULSE_WIDTH_COEF)
					{
						DataTable[REG_QG_I_RESULT] =  0;
						DataTable[REG_QG_RESULT] = 0;
						DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
						DataTable[REG_PROBLEM] = PROBLEM_VG_CUTOFF_NOT_REACHED;
					}
					else
					{
						if(Ig > DataTable[REG_QG_I] * QG_I_THRESHOLD_COEF)
						{
							DataTable[REG_QG_I_RESULT] =  Ig;
							DataTable[REG_QG_RESULT] = Qg;
							DataTable[REG_OP_RESULT] = OPRESULT_OK;
						}
						else
						{
							DataTable[REG_QG_I_RESULT] =  0;
							DataTable[REG_QG_RESULT] = 0;
							DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
							DataTable[REG_PROBLEM] = PROBLEM_CURRENT_NOT_REACHED;
						}
					}

					QG_ResetConfigStageToDefault();
					CONTROL_ResetHardwareToDefaultState();

					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
				else
					CONTROL_SwitchToFault(DF_POWER_CURRENT);
			}
		break;

		case QG_SelfTest:
		case QG_Sertification:
			QG_CalculateData(&PulseWidth, &Ig, &Qg);
			DataTable[REG_QG_I_RESULT] =  Ig;
			DataTable[REG_QG_RESULT] = Qg;
			DataTable[REG_OP_RESULT] = OPRESULT_OK;

			QG_ResetConfigStageToDefault();
			CONTROL_ResetHardwareToDefaultState();

			if(QGMeasureStage == QG_Sertification)
			{
				QGMeasureStage = QG_PreMeasurement;
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			else
			{
				QGMeasureStage = QG_SelfTest;
				CONTROL_SetDeviceState(DS_SelfTest, SS_I_Check);
			}
		break;
	}
}
//-----------------------------------------------

void QG_CalculateData(pFloat32 PulseWidth, pFloat32 Ig, pFloat32 Qg)
{
	LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], VALUES_x_SIZE);
	CONTROL_Values_Counter = VALUES_x_SIZE;
	QG_RemoveDC(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
	QG_Filter(CONTROL_CurrentValues, VALUES_x_SIZE);

	*PulseWidth = QG_ExtractPulseWidth(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
	*Ig = QG_ExtractAverageCurrent(&CONTROL_CurrentValues[0], VALUES_x_SIZE, QG_AVG_LENGTH);
	*Qg = QG_CalculateGateCharge(&CONTROL_CurrentValues[0], VALUES_x_SIZE);
}
//-----------------------------------------------

void QG_ResetConfigStageToDefault()
{
	QgConfigStage = HW_Config;
}
//-----------------------------------------------

float QG_CalculateGateCharge(pFloat32 Buffer, Int16U BufferSize)
{
	float GateCharge = 0;
	float CurrentMax = 0;

	for(int i = 0; i < BufferSize; i++)
	{
		if(CurrentMax < *(Buffer + i))
			CurrentMax = *(Buffer + i);
	}

	for(int i = 0; i < BufferSize; i++)
	{
		if(*(Buffer + i) > CurrentMax * QG_CALC_NOISE_CUT_OFF_COEF)
			GateCharge += *(Buffer + i) * QG_CURRENT_SAMPLE_TIME;
	}

	return GateCharge;
}
//-----------------------------------------------

void QG_RemoveDC(pFloat32 InputArray, Int16U ArraySize)
{
	float DC = 0;

	for(int i = 0; i < QG_DC_PART_STOP_INDEX; i++)
		DC += *(InputArray + i) / QG_DC_PART_STOP_INDEX;

	for(int i = 0; i < ArraySize; i++)
		*(InputArray + i) -= DC;
}
//-----------------------------------------------

float QG_ExtractPulseWidth(pFloat32 Buffer, Int16U BufferSize)
{
	Int16U SampledPointsCounter = 0;

	for (int i = 0; i < BufferSize; i++)
	{
		if(*(Buffer + i) > DataTable[REG_QG_I] * 0.5)
			SampledPointsCounter++;
	}

	return (float)(SampledPointsCounter * QG_CURRENT_SAMPLE_TIME);
}
//-----------------------------------------------

float QG_ExtractAverageCurrent(pFloat32 Buffer, Int16U BufferSize, Int16U AverageLength)
{
	float AverageValue = 0;

	for (int i = 0; i < BufferSize; i++)
		QgCopiedBuffer[i] = *(Buffer + i);

	qsort(QgCopiedBuffer, BufferSize, sizeof(*QgCopiedBuffer), QG_SortCondition);

	for (int i = BufferSize - QG_AVG_IGNORE_POINTS; i > BufferSize - QG_AVG_IGNORE_POINTS - AverageLength; i--)
		AverageValue += *(QgCopiedBuffer + i);

	return (AverageValue / AverageLength);
}
//-----------------------------------------------

int QG_SortCondition(const void *A, const void *B)
{
	return (float)(*(float *)A) - (float)(*(float *)B);
}
//-----------------------------------------

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
