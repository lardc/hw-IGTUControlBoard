// Includes
#include "Measurement.h"
#include "Board.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Global.h"

// Variables
volatile Int16U MEASURE_C_CSenRaw[C_VALUES_x_SIZE];

// Functions
//

Int16U MEASURE_PotSen()
{
	return ADC_Measure(ADC3, ADC3_POT_CHANNEL);
}
//-----------------------------------------------

Int16U MEASURE_V_VSen()
{
	return ADC_Measure(ADC1, ADC1_V_V_SEN_CHANNEL);
}
//-----------------------------------------------

Int16U MEASURE_V_CSen()
{
	return ADC_Measure(ADC1, ADC1_V_C_SEN_CHANNEL);
}
//-----------------------------------------------

Int16U MEASURE_C_VSen()
{
	return ADC_Measure(ADC1, ADC1_C_V_SEN_CHANNEL);
}
//-----------------------------------------------

Int16U MEASURE_C_CSen()
{
	return ADC_Measure(ADC1, ADC1_C_C_SEN_CHANNEL);
}
//-----------------------------------------------

Boolean MEASURE_VGS_Params(volatile RegulatorParamsStruct* Regulator, bool SelfMode)
{
	float V = SelfMode ? CU_V_ADCVToX(MEASURE_V_VSen()) : CU_PotADCVToX(MEASURE_PotSen());
	float C = CU_V_ADCCToX(MEASURE_V_CSen());

	if(Regulator->RegulatorStepCounter == 0)
	{
		V = 0;
		C = 0;
	}
	Regulator->VSen = V;
	Regulator->CSen = C;

	// проверка на достижение током порогового значения
	if((C >= (float)DataTable[REG_VGS_C_TRIG]) && (Regulator->CTrigRegulatorStep == 0))
	{
		Regulator->CTrigRegulatorStep = Regulator->RegulatorStepCounter;
		Regulator->VSenForm[Regulator->RegulatorStepCounter] = (Int16U)V;
		Regulator->CTrigVSen = V;
		Regulator->CTrigCSen = C;
		return true;
	}
	else
		return false;
}
//-----------------------------------------------

void MEASURE_IGES_Params(volatile RegulatorParamsStruct* Regulator, bool SelfMode)
{
	float V = SelfMode ? CU_V_ADCVToX(MEASURE_V_VSen()) : CU_PotADCVToX(MEASURE_PotSen());

	if(Regulator->RegulatorStepCounter == 0)
		V = 0;
	Regulator->VSen = V;
	Regulator->VSenForm[Regulator->RegulatorStepCounter] = V;
}
//-----------------------------------------------

void MEASURE_C_CDMABufferClear()
{
	for(int i = 0; i < C_VALUES_x_SIZE; i++)
		MEASURE_C_CSenRaw[i] = 0;
}
//-----------------------------------------------

Int16U MEASURE_C_DMAExtractCSen()
{
	return MEASURE_Average((Int16U*)&MEASURE_C_CSenRaw[1], C_VALUES_x_SIZE - 1);
}
//-----------------------------------------------

Int16U MEASURE_Average(Int16U* InputArrayAddr, Int16U ArraySize)
{
	Int32U AverageData = 0;

	for(int i = 0; i < ArraySize; i++)
		AverageData += *(InputArrayAddr + i);

	return (Int16U)((float)AverageData / ArraySize);
}
