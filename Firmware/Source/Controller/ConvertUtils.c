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
ConvertParams PotAdcToVSenParams;

// Functions
//
Int16U CU_DACConvert(ConvertParams *Params, float Value);
float CU_ADCConvert(ConvertParams *Params, Int16U Data);
//-----------------------------

Int16U CU_C_VCutoffToExtDAC(float Value)
{
	return CU_DACConvert(&C_VCutoffToExtDACParams, Value);
}
//-----------------------------

Int16U CU_C_VNegativeToExtDAC(float Value)
{
	return CU_DACConvert(&C_VNegativeToExtDACParams, Value);
}
//-----------------------------

Int16U CU_V_VToDAC(float Value)
{
	return CU_DACConvert(&V_VToDACParams, Value);
}
//-----------------------------

Int16U CU_C_CToDAC(float Value)
{
	return CU_DACConvert(&C_CToDACParams, Value);
}
//-----------------------------

float CU_PotADCVToX(Int16U Data)
{
	return CU_ADCConvert(&PotAdcToVSenParams, Data);
}
//-----------------------------

float CU_V_ADCVToX(Int16U Data)
{
	return CU_ADCConvert(&V_AdcToVSenParams, Data);
}
//-----------------------------

float CU_V_ADCCToX(Int16U Data)
{
	return CU_ADCConvert(&V_AdcToCSenParams, Data);
}
//-----------------------------

float CU_C_ADCCToX(Int16U Data)
{
	return CU_ADCConvert(&C_AdcToCSenParams, Data);
}
//-----------------------------

Int16U CU_DACConvert(ConvertParams *Params, float Value)
{
	return (Int16U)(((Value - Params->P0) / Params->P1) * Params->K);
}
//-----------------------------

float CU_ADCConvert(ConvertParams *Params, Int16U Data)
{
	float Value = Data * Params->K;
	return (Value * Value * Params->P2 + Value * Params->P1 + Params->P0);
}
//-----------------------------

void CU_LoadSingleConvertParams(ConvertParams* StructureName, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2)
{
	StructureName->K = (float)DataTable[RegK];
	StructureName->P0 = (float)(Int16S)DataTable[RegP0];
	StructureName->P1 = (float)DataTable[RegP1];
	StructureName->P2 = (RegP2 == 0) ? 0 : (float)((Int16S)DataTable[RegP2]);
}
//-----------------------------

void CU_LoadConvertParams()
{
	CU_LoadSingleConvertParams(&PotAdcToVSenParams, REG_ADC_POT_SEN_K, REG_ADC_POT_SEN_P0, REG_ADC_POT_SEN_P1,
			REG_ADC_POT_SEN_P2);
	CU_LoadSingleConvertParams(&V_AdcToVSenParams, REG_ADC_V_V_SEN_K, REG_ADC_V_V_SEN_P0, REG_ADC_V_V_SEN_P1,
			REG_ADC_V_V_SEN_P2);
	CU_LoadSingleConvertParams(&V_AdcToCSenParams, REG_ADC_V_C_SEN_K, REG_ADC_V_C_SEN_P0, REG_ADC_V_C_SEN_P1,
			REG_ADC_V_C_SEN_P2);
	CU_LoadSingleConvertParams(&C_AdcToCSenParams, REG_ADC_C_C_SEN_K, REG_ADC_C_C_SEN_P0, REG_ADC_C_C_SEN_P1,
			REG_ADC_C_C_SEN_P2);
	CU_LoadSingleConvertParams(&V_VToDACParams, REG_DAC_V_V_SET_K, REG_DAC_V_V_SET_P0, REG_DAC_V_V_SET_P1, 0);
	CU_LoadSingleConvertParams(&C_CToDACParams, REG_DAC_C_C_SET_K, REG_DAC_C_C_SET_P0, REG_DAC_C_C_SET_P1, 0);
	CU_LoadSingleConvertParams(&C_VCutoffToExtDACParams, REG_C_V_CUTOFF_K, REG_C_V_CUTOFF_P0, REG_C_V_CUTOFF_P1, 0);
	CU_LoadSingleConvertParams(&C_VNegativeToExtDACParams, REG_C_V_NEGATIVE_K, REG_C_V_NEGATIVE_P0, REG_C_V_NEGATIVE_P1,
			0);
}
//-----------------------------

