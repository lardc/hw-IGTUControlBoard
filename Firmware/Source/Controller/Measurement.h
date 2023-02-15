#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"

// Types
//
typedef struct __MeasureSample
{
	float Current;
	float Voltage;
}MeasureSample;

// Variables
//
extern volatile Int16U MEASURE_V_CsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_V_VsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_C_Raw[ADC_DMA_BUFF_SIZE_QG];

// Functions
//
MeasureSample MEASURE_SampleVgsIges();
float MEASURE_ExtractAveragedDatas(float* Buffer, Int16U BufferLength);

#endif /* MEASUREMENT_H_ */
