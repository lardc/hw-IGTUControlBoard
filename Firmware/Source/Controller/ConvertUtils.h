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
Int16U CU_UCutoffToExtDAC(float Value);
Int16U CU_UNegativeToExtDAC(float Value);
Int16U CU_UUToDAC(float Value);
Int16U CU_IIToDAC(float Value);
float CU_UADCUToX(Int16U Data);
float CU_UADCIToX(Int16U Data);
float CU_IADCIToX(Int16U Data);
void CU_LoadSingleConvertParams(ConvertParams* StructureName, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2);
void CU_LoadConvertParams();

#endif /* CONVERTUTILS_H_ */
