// Includes
#include "Measurement.h"
#include "Board.h"
#include "DataTable.h"
#include "Global.h"
#include "ConvertUtils.h"
#include "LowLevel.h"

// Variables
volatile Int16U MEASURE_VoltageRaw[ADC_V_DMA_BUFF_SIZE] = {0};
volatile Int16U MEASURE_CurrentRaw[ADC_V_DMA_BUFF_SIZE] = {0};
volatile Int16U MEASURE_Qg_DataRaw[ADC_DMA_BUFF_SIZE_QG] = {0};
//
Int16U CurrentRange = MEASURE_V_I_R2;

// Functions prototypes
//
float MEASURE_ExtractX(volatile Int16U* InputArray, Int16U ArraySize);

// Functions
//
MeasureSample MEASURE_V_SampleVI()
{
	MeasureSample Sample;

	Sample.Voltage = CU_V_ADCtoV(MEASURE_ExtractX(&MEASURE_VoltageRaw[1], ADC_V_DMA_BUFF_SIZE - 1));
	Sample.Current = CU_V_ADCtoI(MEASURE_ExtractX(&MEASURE_CurrentRaw[1], ADC_V_DMA_BUFF_SIZE - 1), CurrentRange);
	MEASURE_StartNewSampling();

	return Sample;
}
//-----------------------------------------------

float MEASURE_ExtractX(volatile Int16U* InputArray, Int16U ArraySize)
{
	Int32U AverageData = 0;

	for(int i = 0; i < ArraySize; i++)
		AverageData += *(InputArray + i);

	return (float)AverageData / ArraySize;
}
//-----------------------------------------------

void MEASURE_StartNewSampling()
{
	DMA_TransferCompleteReset(DMA1, DMA_TRANSFER_COMPLETE);
	DMA_TransferCompleteReset(DMA2, DMA_TRANSFER_COMPLETE);
	ADC_SamplingStart(ADC1);
	ADC_SamplingStart(ADC3);
}
//-----------------------------------------------

float MEASURE_ExtractAveragedDatas(float* Buffer, Int16U BufferLength)
{
	float Temp = 0;

	for(int i = 0; i < BufferLength; i++)
		Temp += *(Buffer + i) / BufferLength;

	return Temp;
}
//-----------------------------

void MEASURE_ResetDMABuffers()
{
	for(int i = 0; i < ADC_V_DMA_BUFF_SIZE; i++)
	{
		MEASURE_VoltageRaw[i] = 0;
		MEASURE_CurrentRaw[i] = 0;
	}

	for(int i = 0; i < ADC_DMA_BUFF_SIZE_QG; i++)
		MEASURE_Qg_DataRaw[i] = 0;
}
//-----------------------------

Int16U MEASURE_V_SetCurrentRange(float Current)
{
	if(Current <= THRESHOLD_V_I_R0)
	{
		CurrentRange = MEASURE_V_I_R0;
		LL_V_IlimLowRange();
	}
	else
	{
		if(Current > THRESHOLD_V_I_R1)
		{
			CurrentRange = MEASURE_V_I_R2;
			LL_V_IsenseHighRange1();
		}
		else
		{
			CurrentRange = MEASURE_V_I_R1;
			LL_V_IsenseHighRange0();
		}

		LL_V_IlimHighRange();
	}

	return CurrentRange;
}
//-----------------------------
