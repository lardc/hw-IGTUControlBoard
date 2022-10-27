// Includes
#include "Measurement.h"
#include "Board.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Global.h"

// Variables
volatile Int16U MEASURE_ADC_IGateRaw[I_VALUES_x_SIZE];

// Functions
//

Int16U MEASURE_UUSen()
{
	Int16U result = ADC_Measure(ADC3, ADC3_U_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Int16U MEASURE_UISen()
{
	Int16U result = ADC_Measure(ADC1, ADC1_I_SEN_CHANNEL);
	return result;
}
//-----------------------------------------------

Int16U MEASURE_IIGate()
{
	Int16U result = ADC_Measure(ADC1, ADC1_I_GATE_CHANNEL);
	return result;
}
//-----------------------------------------------

Boolean MEASURE_UParams(volatile RegulatorParamsStruct* Regulator)
{
	float U = CU_UADCUToX(MEASURE_UUSen());
	float I = CU_UADCIToX(MEASURE_UISen());

	if(Regulator->RegulatorPulseCounter == 0)
	{
		U = 0;
		I = 0;
	}
	Regulator->UMeasured = U;
	Regulator->UFormMeasured[Regulator->RegulatorPulseCounter] = U;
	Regulator->IFormMeasured[Regulator->RegulatorPulseCounter] = I;

	// проверка на достижение током порогового значения
	if ((I >= (float)DataTable[REG_U_I_TRIG]) && (Regulator->ITrigRegulatorPulse == 0))
	{
		Regulator->ITrigRegulatorPulse = Regulator->RegulatorPulseCounter;
		return true;
	}
	else return false;
}
//-----------------------------------------------

void MEASURE_DMAIGateBufferClear()
{
	for(int i = 0; i < I_VALUES_x_SIZE; i++)
		MEASURE_ADC_IGateRaw[i] = 0;
}
//-----------------------------------------------

/*Int16U MEASURE_DMAExtractIGate()
{
	return MEASURE_Average(&MEASURE_ADC_IGateRaw[1], I_VALUES_x_SIZE - 1);
}*/
//-----------------------------------------------

Int16U MEASURE_Average(Int16U* InputArray, Int16U ArraySize)
{
	Int32U AverageData = 0;

	for(int i = 0; i < ArraySize; i++)
		AverageData += *(InputArray + i);

	return (Int16U)((float)AverageData / ArraySize);
}
