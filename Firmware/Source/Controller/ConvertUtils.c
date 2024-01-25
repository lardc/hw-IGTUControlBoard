// Includes
//
#include "ConvertUtils.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Measurement.h"

// Variables
ConvertParams V_ADCtoVParams;
ConvertParams V_ADCtoIParams[MEASURE_V_I_RANGES];
ConvertParams V_VtoDACParams;
//
ConvertParams I_ItoDACParams;
ConvertParams I_VCutoffToDACParams;
ConvertParams I_VnegativeToDACParams;
ConvertParams I_ADCtoVParams[MEASURE_I_V_TYPES];
ConvertParams I_ADCtoIParams;

// Functions
//
Int16U CU_XtoDAC(ConvertParams *Params, float Value);
float CU_ADCtoX(ConvertParams *Params, Int16U Data);
//-----------------------------
//
Int16U CU_I_VcutoffToDAC(float Value)
{
	return (Value) ? CU_XtoDAC(&I_VCutoffToDACParams, Value) : 0;
}
//-----------------------------

Int16U CU_I_VnegativeToDAC(float Value)
{
	return (Value) ? CU_XtoDAC(&I_VnegativeToDACParams, Value) : 0;
}
//-----------------------------

Int16U CU_V_VtoDAC(float Value)
{
	return (Value) ? CU_XtoDAC(&V_VtoDACParams, Value) : 0;
}
//-----------------------------

Int16U CU_I_ItoDAC(float Value)
{
	return (Value) ? CU_XtoDAC(&I_ItoDACParams, Value) : 0;
}
//-----------------------------

float CU_V_ADCtoV(Int16U Data)
{
	return CU_ADCtoX(&V_ADCtoVParams, Data);
}
//-----------------------------

float CU_V_ADCtoI(Int16U Data, Int16U CurrentRange)
{
	return CU_ADCtoX(&V_ADCtoIParams[CurrentRange], Data);
}
//-----------------------------

float CU_I_ADCtoI(Int16U Data)
{
	return CU_ADCtoX(&I_ADCtoIParams, Data);
}
//-----------------------------

float CU_I_ADCtoV(Int16U Data, Int16U Vtype)
{
	return CU_ADCtoX(&I_ADCtoVParams[Vtype], Data);
}
//-----------------------------
//
Int16U CU_XtoDAC(ConvertParams *Params, float Value)
{
	float Result = Value * Value * Params->P2 + Value * Params->P1 + Params->P0;
	Result = (Int16U)(Result * Params->K + Params->B);

	return (Result > DAC_MAX_VALUE) ? DAC_MAX_VALUE : Result;
}
//-----------------------------

float CU_ADCtoX(ConvertParams *Params, Int16U Data)
{
	float Value = Data * Params->K + Params->B;
	return (Value * Value * Params->P2 + Value * Params->P1 + Params->P0);
}
//-----------------------------

void CU_LoadSingleParams(ConvertParams* Params, Int16U RegB, Int16U RegK, Int16U RegP0, Int16U RegP1, Int16U RegP2)
{
	Params->B = DataTable[RegB];
	Params->K = DataTable[RegK];
	Params->P0 = DataTable[RegP0];
	Params->P1 = DataTable[RegP1];
	Params->P2 = DataTable[RegP2];
}
//-----------------------------

void CU_LoadConvertParams()
{
	CU_LoadSingleParams(&V_ADCtoVParams, REG_V_ADC_TO_V_B, REG_V_ADC_TO_V_K, REG_V_ADC_TO_V_P0, REG_V_ADC_TO_V_P1, REG_V_ADC_TO_V_P2);
	CU_LoadSingleParams(&V_ADCtoIParams[MEASURE_V_I_R0], REG_V_ADC_TO_I_R0_B, REG_V_ADC_TO_I_R0_K, REG_V_ADC_TO_I_R0_P0, REG_V_ADC_TO_I_R0_P1, REG_V_ADC_TO_I_R0_P2);
	CU_LoadSingleParams(&V_ADCtoIParams[MEASURE_V_I_R1], REG_V_ADC_TO_I_R1_B, REG_V_ADC_TO_I_R1_K, REG_V_ADC_TO_I_R1_P0, REG_V_ADC_TO_I_R1_P1, REG_V_ADC_TO_I_R1_P2);
	CU_LoadSingleParams(&V_ADCtoIParams[MEASURE_V_I_R2], REG_V_ADC_TO_I_R2_B, REG_V_ADC_TO_I_R2_K, REG_V_ADC_TO_I_R2_P0, REG_V_ADC_TO_I_R2_P1, REG_V_ADC_TO_I_R2_P2);
	CU_LoadSingleParams(&V_VtoDACParams, REG_V_V_TO_DAC_B, REG_V_V_TO_DAC_K, REG_V_V_TO_DAC_P0, REG_V_V_TO_DAC_P1, REG_V_V_TO_DAC_P2);
	CU_LoadSingleParams(&I_ItoDACParams, REG_I_I_TO_DAC_B, REG_I_I_TO_DAC_K, REG_I_I_TO_DAC_P0, REG_I_I_TO_DAC_P1, REG_I_I_TO_DAC_P2);
	CU_LoadSingleParams(&I_VCutoffToDACParams, REG_I_VC_TO_DAC_B, REG_I_VC_TO_DAC_K, REG_I_VC_TO_DAC_P0, REG_I_VC_TO_DAC_P1, REG_I_VC_TO_DAC_P2);
	CU_LoadSingleParams(&I_VnegativeToDACParams, REG_I_VN_TO_DAC_B, REG_I_VN_TO_DAC_K, REG_I_VN_TO_DAC_P0, REG_I_VN_TO_DAC_P1, REG_I_VN_TO_DAC_P2);
	CU_LoadSingleParams(&I_ADCtoVParams[MEASURE_I_VC], REG_I_ADC_TO_VC_B, REG_I_ADC_TO_VC_K, REG_I_ADC_TO_VC_P0, REG_I_ADC_TO_VC_P1, REG_I_ADC_TO_VC_P2);
	CU_LoadSingleParams(&I_ADCtoVParams[MEASURE_I_VN], REG_I_ADC_TO_VN_B, REG_I_ADC_TO_VN_K, REG_I_ADC_TO_VN_P0, REG_I_ADC_TO_VN_P1, REG_I_ADC_TO_VN_P2);
	CU_LoadSingleParams(&I_ADCtoIParams, REG_I_ADC_TO_I_B, REG_I_ADC_TO_I_K, REG_I_ADC_TO_I_P0, REG_I_ADC_TO_I_P1, REG_I_ADC_TO_I_P2);
}
//-----------------------------

