// Includes
#include "Measurement.h"
#include "Board.h"
#include "DataTable.h"
#include "Global.h"
#include "ConvertUtils.h"

// Variables
volatile Int16U MEASURE_VoltageRaw[ADC_DMA_BUFF_SIZE_VGS_IGES] = {0};
volatile Int16U MEASURE_CurrentRaw[ADC_DMA_BUFF_SIZE_VGS_IGES] = {0};
volatile Int16U MEASURE_Qg_DataRaw[ADC_DMA_BUFF_SIZE_QG] = {0};

// Functions prototypes
//
float MEASURE_ExtractX(volatile Int16U* InputArray, Int16U ArraySize);
void MEASURE_StartNewSampling();

// Functions
//
MeasureSample MEASURE_SampleVgsIges()
{
	MeasureSample Sample;

	Sample.Voltage = CU_V_ADCtoV(MEASURE_ExtractX(&MEASURE_VoltageRaw[0], ADC_DMA_BUFF_SIZE_VGS_IGES - 1));
	Sample.Current = CU_V_ADCtoI(MEASURE_ExtractX(&MEASURE_CurrentRaw[0], ADC_DMA_BUFF_SIZE_VGS_IGES - 1));
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
