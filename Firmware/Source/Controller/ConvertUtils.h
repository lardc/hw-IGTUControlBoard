#ifndef CONVERTUTILS_H_
#define CONVERTUTILS_H_

// Includes
#include "ZwBase.h"

// Structs
typedef struct __ConvertParams
{
	float P2;
	float P1;
	float P0;
	float K;
}ConvertParams;

// Functions
Int16U CU_C_VCutoffToExtDAC(float Value);
Int16U CU_C_VNegativeToExtDAC(float Value);
Int16U CU_V_VToDAC(float Value);
Int16U CU_C_CToDAC(float Value);
float CU_V_ADCVToX(Int16U Data);
float CU_V_ADCCToX(Int16U Data);
float CU_C_ADCCToX(Int16U Data);
void CU_LoadSingleConvertParams(ConvertParams* StructureName, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2);
void CU_LoadConvertParams();

#endif /* CONVERTUTILS_H_ */
