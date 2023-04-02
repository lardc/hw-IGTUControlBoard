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

// Definitions
//
#define QG_AVG_START_INDEX			20
#define QG_AVG_LENGTH				20
//
#define PAU_WAIT_READY_TIMEOUT		5000
#define QG_FILTER_COEFFICIENT		0.1

// Types
//
typedef enum __QgPrepareStage
{
	TOCUHP_Config = 0,
	TOCUHP_Wating = 1,
	HW_Config = 2
} QgPrepareStage;

// Variables
//
QgPrepareStage QgConfigStage = TOCUHP_Config;
Int64U TOCUHP_StateTimeout = 0;

// Function prototypes
//
void QG_CacheVariables();
float QG_CalculateGateCharge(pFloat32 Buffer, Int16U BufferSize);
void QG_SetPulseWidth(float Period);
void QG_Filter(pFloat32 InputArray, Int16U ArraySize);
float QG_ExtractAverageCurrent(pFloat32 Buffer, Int16U BufferSize);

// Functions
//
void QG_Prepare()
{
	if(!TOCUHP_ReadState(&TOCUHP_State))
		CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);

	if(TOCUHP_IsEmulatedState())
		QgConfigStage = HW_Config;

	switch(QgConfigStage)
	{
		case TOCUHP_Config:
			if(TOCUHP_State == TOCUHP_DS_Ready || CONTROL_State == DS_SelfTest)
			{
				if(TOCUHP_Configure(DataTable[REG_QG_V_POWER], DataTable[REG_QG_I_POWER]))
				{
					TOCUHP_StateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
					QgConfigStage = TOCUHP_Wating;
				}
				else
				{
					CONTROL_ResetHardwareToDefaultState();
					CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
				}
			}
			else
			{
				CONTROL_ResetHardwareToDefaultState();
				CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
			}
			break;

		case TOCUHP_Wating:
			if(TOCUHP_State == TOCUHP_DS_Ready)
				QgConfigStage = HW_Config;
			else
			{
				if(CONTROL_TimeCounter >= TOCUHP_StateTimeout)
				{
					DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
					CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
				}
			}
			break;

		case HW_Config:
			QG_CacheVariables();

			INITCFG_ConfigADC_Qg_I();
			INITCFG_ConfigDMA_Qg(VALUES_x_SIZE2);
			MEASURE_ResetDMABuffers();
			QG_SetPulseWidth(DataTable[REG_QG_I_DURATION]);

			LL_ExDACVCutoff(CU_I_VcutoffToDAC(DataTable[REG_QG_V_CUTOFF]));
			LL_ExDACVNegative(CU_I_VnegativeToDAC(DataTable[REG_QG_V_NEGATIVE]));
			DELAY_MS(50);
			LL_I_SetDAC(CU_I_ItoDAC(DataTable[REG_QG_I]));
			LL_I_Start(false);
			LL_I_Enable(true);
			DELAY_MS(20);

			CONTROL_SwitchOutMUX(Current);

			CONTROL_SetDeviceState(CONTROL_State, SS_QgProcess);
			MEASURE_StartNewSampling();
			QG_Pulse(true);

			TOCUHP_StateTimeout = CONTROL_TimeCounter + PAU_WAIT_READY_TIMEOUT;
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
	INITCFG_ConfigTimer3(Period);
}
//----------------------------

void QG_Pulse(bool State)
{
	LL_SyncOSC(State);
	LL_SyncTOCUHP(State);

	(State) ? TIM_Start(TIM3) : TIM_Stop(TIM3);
	LL_I_Start(State);
}
//----------------------------

void QG_SaveResult()
{
	if(!TOCUHP_ReadState(&TOCUHP_State))
		CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);

	if(TOCUHP_IsEmulatedState())
		TOCUHP_State = TOCUHP_DS_Ready;

	if(TOCUHP_State == TOCUHP_DS_Fault || TOCUHP_State == TOCUHP_DS_Disabled || TOCUHP_State == TOCUHP_DS_None)
		CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
	else
	{
		if(TOCUHP_State == TOCUHP_DS_Ready || CONTROL_State == DS_SelfTest)
		{
			if(DMA_IsTransferComplete(DMA1, DMA_ISR_TCIF1))
			{
				LOG_CopyCurrentToEndpoints(&CONTROL_CurrentValues[0], &MEASURE_Qg_DataRaw[0], ADC_DMA_BUFF_SIZE_QG / 2, 0);
				CONTROL_Values_Counter = VALUES_x_SIZE2;

				QG_Filter(&CONTROL_CurrentValues[0], VALUES_x_SIZE2);

				DataTable[REG_QG_I_RESULT] =  QG_ExtractAverageCurrent(&CONTROL_CurrentValues[0], VALUES_x_SIZE2);
				DataTable[REG_QG_RESULT] = QG_CalculateGateCharge(&CONTROL_CurrentValues[0], VALUES_x_SIZE2);
				DataTable[REG_OP_RESULT] = OPRESULT_OK;

				if(CONTROL_State == DS_InProcess)
					CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
		}
		else
		{
			if(CONTROL_TimeCounter >= TOCUHP_StateTimeout)
			{
				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_TOCUHP_WRONG_STATE);
			}
		}
	}
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
	float CurrentMax = 0;
	float Sum = 0;
	Int16U Counter = 0;

	// Поиск максимума
	for(int i = 0; i < BufferSize; i++)
	{
		if(*(Buffer + i) > CurrentMax)
			CurrentMax = *(Buffer + i);
	}

	// Усреднение
	for(int i = 0; i < BufferSize; i++)
	{
		if(*(Buffer + i) > CurrentMax * 0.7)
		{
			Sum += *(Buffer + i);
			Counter++;
		}
	}

	return Sum / Counter;
}
//-----------------------------------------------
