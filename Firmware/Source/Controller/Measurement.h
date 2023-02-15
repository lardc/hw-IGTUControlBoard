#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"
#include "ConvertUtils.h"
#include "Regulator.h"

#define ADC_NUMBER_OF_CHANNELS		2
// Variables
extern volatile Int16U MEASURE_V_CsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_V_VsensRaw[ADC_DMA_BUFF_SIZE_VGS_IGES];
extern volatile Int16U MEASURE_C_Raw[ADC_DMA_BUFF_SIZE_QG];

// Functions
Int16U MEASURE_V_VSen();
Int16U MEASURE_V_CSen();
Int16U MEASURE_C_CSen();
Boolean MEASURE_VGS_Params(volatile RegulatorParamsStruct* Regulator, Int16U State);
void MEASURE_IGES_Params(volatile RegulatorParamsStruct* Regulator, Int16U State);
Int16U MEASURE_Average(Int16U* InputArrayAddr, Int16U ArraySize, Int16U NumberOfChannels, Int16U Channel);
void MEASURE_C_DMABufferClear();
Int16U MEASURE_C_DMAExtractCSen();
Int16U MEASURE_C_DMAExtractVSen();

#endif /* MEASUREMENT_H_ */
