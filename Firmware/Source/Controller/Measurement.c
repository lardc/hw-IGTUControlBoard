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

Int16U MEASURE_V_VSen()
{
	Int16U result = ADC_Measure(ADC1, ADC1_V_V_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Int16U MEASURE_V_CSen()
{
	Int16U result = ADC_Measure(ADC1, ADC1_V_C_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Int16U MEASURE_C_VSen()
{
	Int16U result = ADC_Measure(ADC1, ADC1_C_V_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Int16U MEASURE_C_CSen()
{
	Int16U result = ADC_Measure(ADC1, ADC1_C_C_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Boolean MEASURE_V_VParams(volatile RegulatorParamsStruct* Regulator)
{
	float V = CU_V_ADCVToX(MEASURE_V_VSen());
	float C = CU_V_ADCCToX(MEASURE_V_CSen());

	if(Regulator->RegulatorPulseCounter == 0)
	{
		V = 0;
		C = 0;
	}
	Regulator->VSen = V;
	Regulator->VSenForm[Regulator->RegulatorPulseCounter] = V;
	Regulator->CSenForm[Regulator->RegulatorPulseCounter] = C;

	// проверка на достижение током порогового значения
	if ((C >= (float)DataTable[REG_VGS_C_TRIG]) && (Regulator->CTrigRegulatorPulse == 0))
	{
		Regulator->CTrigRegulatorPulse = Regulator->RegulatorPulseCounter;
		return true;
	}
	else return false;
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
