// Includes
//
#include "ConvertUtils.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Global.h"


// Variables
ConvertParams AdcToUUSenParams;
ConvertParams AdcToUISenParams;
ConvertParams AdcToIIGateParams;
ConvertParams UUToDACParams;
ConvertParams IIToDACParams;
ConvertParams UCutoffToExtDACParams;
ConvertParams UNegativeToExtDACParams;

// Functions
//
Int16U CU_UCutoffToExtDAC(float Value)
{
	return (Int16U)(((Value - UCutoffToExtDACParams.P0) / UCutoffToExtDACParams.P1) * UCutoffToExtDACParams.K);
}
//-----------------------------

Int16U CU_UNegativeToExtDAC(float Value)
{
	return (Int16U)(((Value - UNegativeToExtDACParams.P0) / UNegativeToExtDACParams.P1) * UNegativeToExtDACParams.K);
}
//-----------------------------

Int16U CU_UUToDAC(float Value)
{
	return (Int16U)(((Value - UUToDACParams.P0) / UUToDACParams.P1) * UUToDACParams.K);
}
//-----------------------------

Int16U CU_IIToDAC(float Value)
{
	return (Int16U)(((Value - IIToDACParams.P0) / IIToDACParams.P1) * IIToDACParams.K);
}
//-----------------------------

float CU_UADCUToX(Int16U Data)
{
	float Value = Data * AdcToUUSenParams.K;
	return (Value * Value * AdcToUUSenParams.P2 + Value * AdcToUUSenParams.P1 + AdcToUUSenParams.P0);
}
//-----------------------------

float CU_UADCIToX(Int16U Data)
{
	float Value = Data * AdcToUISenParams.K;
	return (Value * Value * AdcToUISenParams.P2 + Value * AdcToUISenParams.P1 + AdcToUISenParams.P0);
}
//-----------------------------

float CU_IADCIToX(Int16U Data)
{
	float Value = Data * AdcToIIGateParams.K;
	return (Value * Value * AdcToIIGateParams.P2 + Value * AdcToIIGateParams.P1 + AdcToIIGateParams.P0);
}
//-----------------------------

void CU_LoadSingleConvertParams(ConvertParams* StructureName, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2)
{
	StructureName->K = (float)DataTable[RegK] / 1000;
	StructureName->P0 = (float)(Int16S)DataTable[RegP0];
	StructureName->P1 = (float)DataTable[RegP1] / 1000;
	StructureName->P2 = (RegP2 == 0) ? 0 : (float)((Int16S)DataTable[RegP2]) / 1e6;
}

void CU_LoadConvertParams()
{
	CU_LoadSingleConvertParams(&AdcToUUSenParams, REG_ADC_U_U_SEN_K, REG_ADC_U_U_SEN_P0, REG_ADC_U_U_SEN_P1, REG_ADC_U_U_SEN_P2);
	CU_LoadSingleConvertParams(&AdcToUISenParams, REG_ADC_U_I_SEN_K, REG_ADC_U_I_SEN_P0, REG_ADC_U_I_SEN_P1, REG_ADC_U_I_SEN_P2);
	CU_LoadSingleConvertParams(&AdcToIIGateParams, REG_ADC_I_I_GATE_K, REG_ADC_I_I_GATE_P0, REG_ADC_I_I_GATE_P1, REG_ADC_I_I_GATE_P2);
	CU_LoadSingleConvertParams(&UUToDACParams, REG_DAC_U_U_K, REG_DAC_U_U_P0, REG_DAC_U_U_P1, 0);
	CU_LoadSingleConvertParams(&IIToDACParams, REG_DAC_I_I_K, REG_DAC_I_I_P0, REG_DAC_I_I_P1, 0);
	CU_LoadSingleConvertParams(&UCutoffToExtDACParams, REG_EXT_DAC_I_CUTOFF_K, REG_EXT_DAC_I_CUTOFF_P0, REG_EXT_DAC_I_CUTOFF_P1, 0);
	CU_LoadSingleConvertParams(&UNegativeToExtDACParams, REG_EXT_DAC_I_NEGATIVE_K, REG_EXT_DAC_I_NEGATIVE_P0, REG_EXT_DAC_I_NEGATIVE_P1, 0);
}
//-----------------------------

