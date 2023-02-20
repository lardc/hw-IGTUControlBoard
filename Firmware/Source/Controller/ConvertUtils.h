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
	float B;
} ConvertParams;

// Functions
Int16U CU_I_VcutoffToDAC(float Value);
Int16U CU_I_VnegativeToDAC(float Value);
Int16U CU_V_VtoDAC(float Value);
Int16U CU_I_ItoDAC(float Value);
float CU_V_ADCtoV(Int16U Data);
float CU_V_ADCtoI(Int16U Data, Int16U CurrentRange);
float CU_I_ADCtoI(Int16U Data);
float CU_I_ADCtoV(Int16U Data);
void CU_LoadSingleParams(ConvertParams* Params, Int16U RegB, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2);
void CU_LoadConvertParams();

#endif /* CONVERTUTILS_H_ */
