#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"

// Definitions
//
#define MEASURE_V_I_RANGES				3		// Количество диапазонов измерения тока в источнике напряжения
#define MEASURE_V_I_R0					0
#define MEASURE_V_I_R1					1
#define MEASURE_V_I_R2					2
//
#define MEASURE_I_V_TYPES				2
#define MEASURE_I_VC					0
#define MEASURE_I_VN					1

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
Int16U MEASURE_V_SetCurrentRange(float Current);
void MEASURE_StartNewSampling();

#endif /* MEASUREMENT_H_ */
