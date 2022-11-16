// Includes
//
#include "ConvertUtils.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Global.h"


// Variables
ConvertParams V_AdcToVSenParams;
ConvertParams V_AdcToCSenParams;
ConvertParams C_AdcToCSenParams;
ConvertParams V_VToDACParams;
ConvertParams C_CToDACParams;
ConvertParams C_VCutoffToExtDACParams;
ConvertParams C_VNegativeToExtDACParams;

// Functions
//
Int16U CU_C_VCutoffToExtDAC(float Value)
{
	return (Int16U)(((Value - C_VCutoffToExtDACParams.P0) / C_VCutoffToExtDACParams.P1) * C_VCutoffToExtDACParams.K);
}
//-----------------------------

Int16U CU_C_VNegativeToExtDAC(float Value)
{
	return (Int16U)(((Value - C_VNegativeToExtDACParams.P0) / C_VNegativeToExtDACParams.P1) * C_VNegativeToExtDACParams.K);
}
//-----------------------------

Int16U CU_V_VToDAC(float Value)
{
	return (Int16U)(((Value - V_VToDACParams.P0) / V_VToDACParams.P1) * V_VToDACParams.K);
}
//-----------------------------

Int16U CU_C_CToDAC(float Value)
{
	return (Int16U)(((Value - C_CToDACParams.P0) / C_CToDACParams.P1) * C_CToDACParams.K);
}
//-----------------------------

float CU_V_ADCVToX(Int16U Data)
{
	float Value = Data * V_AdcToVSenParams.K;
	return (Value * Value * V_AdcToVSenParams.P2 + Value * V_AdcToVSenParams.P1 + V_AdcToVSenParams.P0);
}
//-----------------------------

float CU_V_ADCCToX(Int16U Data)
{
	float Value = Data * V_AdcToCSenParams.K;
	return (Value * Value * V_AdcToCSenParams.P2 + Value * V_AdcToCSenParams.P1 + V_AdcToCSenParams.P0);
}
//-----------------------------

float CU_C_ADCCToX(Int16U Data)
{
	float Value = Data * C_AdcToCSenParams.K;
	return (Value * Value * C_AdcToCSenParams.P2 + Value * C_AdcToCSenParams.P1 + C_AdcToCSenParams.P0);
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
	CU_LoadSingleConvertParams(&V_AdcToVSenParams, REG_ADC_V_V_SEN_K, REG_ADC_V_V_SEN_P0, REG_ADC_V_V_SEN_P1, REG_ADC_V_V_SEN_P2);
	CU_LoadSingleConvertParams(&V_AdcToCSenParams, REG_ADC_V_C_SEN_K, REG_ADC_V_C_SEN_P0, REG_ADC_V_C_SEN_P1, REG_ADC_V_C_SEN_P2);
	CU_LoadSingleConvertParams(&C_AdcToCSenParams, REG_ADC_C_C_SEN_K, REG_ADC_C_C_SEN_P0, REG_ADC_C_C_SEN_P1, REG_ADC_C_C_SEN_P2);
	CU_LoadSingleConvertParams(&V_VToDACParams, REG_DAC_V_V_SET_K, REG_DAC_V_V_SET_P0, REG_DAC_V_V_SET_P1, 0);
	CU_LoadSingleConvertParams(&C_CToDACParams, REG_DAC_C_C_SET_K, REG_DAC_C_C_SET_P0, REG_DAC_C_C_SET_P1, 0);
	CU_LoadSingleConvertParams(&C_VCutoffToExtDACParams, REG_C_V_CUTOFF_K, REG_C_V_CUTOFF_P0, REG_C_V_CUTOFF_P1, 0);
	CU_LoadSingleConvertParams(&C_VNegativeToExtDACParams, REG_C_V_NEGATIVE_K, REG_C_V_NEGATIVE_P0, REG_C_V_NEGATIVE_P1, 0);
}
//-----------------------------

