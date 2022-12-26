#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

// Includes
#include "SysConfig.h"
#include "ZwBase.h"
#include "ConvertUtils.h"
#include "Regulator.h"

// Variables
extern volatile Int16U MEASURE_C_CSenRaw[C_VALUES_x_SIZE];

// Functions
Int16U MEASURE_V_VSen();
Int16U MEASURE_V_CSen();
Int16U MEASURE_C_CSen();
Boolean MEASURE_VGS_Params(volatile RegulatorParamsStruct* Regulator, bool SelfMode);
void MEASURE_IGES_Params(volatile RegulatorParamsStruct* Regulator, bool SelfMode);
Int16U MEASURE_Average(Int16U* InputArray, Int16U ArraySize);
void MEASURE_C_CDMABufferClear();
Int16U MEASURE_C_DMAExtractCSen();

#endif /* MEASUREMENT_H_ */
