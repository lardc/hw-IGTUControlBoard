#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"

// Definitions
//
#define MEASURE_V_I_RANGES				2		// Количество диапазонов измерения тока в источнике напряжения

// Types
//
typedef struct __MeasureSample
{
	float Current;
	float Voltage;
}MeasureSample;

// Variables
//
extern volatile Int16U MEASURE_VoltageRaw[ADC_V_DMA_BUFF_SIZE];
extern volatile Int16U MEASURE_CurrentRaw[ADC_V_DMA_BUFF_SIZE];
extern volatile Int16U MEASURE_Qg_DataRaw[ADC_DMA_BUFF_SIZE_QG];

// Functions
//
MeasureSample MEASURE_V_SampleVI();
float MEASURE_ExtractAveragedDatas(float* Buffer, Int16U BufferLength);
void MEASURE_ResetDMABuffers();
void MEASURE_V_SetCurrentRange(float Current);
void MEASURE_StartNewSampling();

#endif /* MEASUREMENT_H_ */
