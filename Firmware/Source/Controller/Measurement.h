#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"
#include "ConvertUtils.h"
#include "Regulator.h"

// Variables
extern volatile Int16U MEASURE_ADC_IGateRaw[I_VALUES_x_SIZE];

// Functions
Int16U MEASURE_UUSen();
Int16U MEASURE_UISen();
Int16U MEASURE_IIGate();
Boolean MEASURE_UParams(volatile RegulatorParamsStruct* Regulator);
//Int16U MEASURE_Average(Int16U InputArray, Int16U ArraySize);
void MEASURE_DMAIGateBufferClear();
Int16U MEASURE_DMAExtractIGate();

#endif /* MEASUREMENT_H_ */
