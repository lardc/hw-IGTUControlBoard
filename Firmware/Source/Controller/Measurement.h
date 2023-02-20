﻿#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"

// Definitions
//
#define MEASURE_BUFFERS_SIZE		ADC_DMA_BUFF_SIZE_QG
//
#define MEASURE_IGES_MAX			0.3	// мА

// Types
//
typedef struct __MeasureSample
{
	float Current;
	float Voltage;
}MeasureSample;

// Variables
//
extern volatile Int16U MEASURE_VoltageRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_CurrentRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_Qg_DataRaw[ADC_DMA_BUFF_SIZE_QG];

// Functions
//
MeasureSample MEASURE_SampleVgsIges();
float MEASURE_ExtractAveragedDatas(float* Buffer, Int16U BufferLength);
void MEASURE_ResetDMABuffers();
void MEASURE_V_SetCurrentRange(float Current);

#endif /* MEASUREMENT_H_ */
