// Includes
#include "Measurement.h"
#include "Board.h"
#include "DataTable.h"
#include "Global.h"

// Variables
volatile Int16U MEASURE_V_CsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES] = {0};
volatile Int16U MEASURE_V_VsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES] = {0};
volatile Int16U MEASURE_C_Raw[ADC_DMA_BUFF_SIZE_QG] = {0};

// Functions prototypes
//
float MEASURE_ExtractX(Int16U* InputArray, Int16U ArraySize);
float MEASURE_ExtractVgsIges(Int16U* InputArray);
void MEASURE_StartNewSampling();

// Functions
//
MeasureSample MEASURE_SampleVgsIges()
{
	MeasureSample Sample;

	Sample.Voltage = CU_ADCtoV(MEASURE_DMAExtract(&MEASURE_V_VsensRaw[0]));
	Sample.Current = CU_ADCtoI(MEASURE_DMAExtract(&MEASURE_V_CsensRaw[0]));
	MEASURE_StartNewSampling();

	return Sample;
}
//-----------------------------------------------

float MEASURE_ExtractX(Int16U* InputArray, Int16U ArraySize)
{
	Int32U AverageData = 0;

	for(int i = 0; i < ArraySize; i++)
		AverageData += *(InputArray + i);

	return (float)AverageData / ArraySize;
}
//-----------------------------------------------

float MEASURE_ExtractVgsIges(Int16U* InputArray)
{
	return MEASURE_DMAExtractX(InputArray, ADC_DMA_BUFF_SIZE_VGS_IGES - 1);
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
