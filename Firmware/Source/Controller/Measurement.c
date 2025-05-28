// Header
#include "Measurement.h"

// Include
#include "DataTable.h"
#include "Board.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

// Definitions
//
#define ADC_RESOLUTION 4095

// Forward functions
void MEASURE_ConvertADCtoValx(pFloat32 InputArray, Int16U DataLength, Int16U RegisterOffset,
		Int16U RegisterK, Int16U RegisterP0, Int16U RegisterP1, Int16U RegisterP2, float RShunt);
float MEASURE_ConvertX(Int16U SampleADC, Int16U RegisterP2, Int16U RegisterP1, Int16U RegisterP0, Int16U RegisterK, Int16U RegisterB);

// Functions
//
float MEASURE_ConvertX(Int16U SampleADC, Int16U RegisterP2, Int16U RegisterP1, Int16U RegisterP0, Int16U RegisterK, Int16U RegisterB)
{
	float Result = ((float)SampleADC / ADC_RESOLUTION) * DataTable[REG_U_ADC_REF] * DataTable[RegisterK] + DataTable[RegisterB];
	Result = Result * Result * DataTable[RegisterP2] + Result * DataTable[RegisterP1] + DataTable[RegisterP0];

	return (Result > 0) ? Result : 0;
}
//------------------------------------

float MEASURE_U1(Int16U SampleADC)
{
	return MEASURE_ConvertX(SampleADC, REG_U_1_P2, REG_U_1_P1, REG_U_1_P0, REG_U_1_K, 
			REG_U_1_B);
}
//------------------------------------

float MEASURE_U2(Int16U SampleADC)
{
	return MEASURE_ConvertX(SampleADC, REG_U_2_P2, REG_U_2_P1, REG_U_2_P0, REG_U_2_K, 
			REG_U_2_B);
}
//------------------------------------

float MEASURE_I(Int16U SampleADC, IChannel Channel)
{
	Int16U offset = 0;

	switch (Channel)
	{
		case I_CHANNEL_1:
			offset = 0;
			break;
		case I_CHANNEL_2:
			offset = 6;
			break;
		case I_CHANNEL_3:
			offset = 12;
			break;
		case I_CHANNEL_4:
			offset = 18;
			break;
		case I_CHANNEL_5:
			offset = 24;
			break;
		case I_CHANNEL_6:
			offset = 30;
			break;
		case I_CHANNEL_7:
			offset = 36;
			break;
		case I_CHANNEL_8:
			offset = 42;
			break;
	}
	return MEASURE_ConvertX(SampleADC, REG_I_1_P2 + offset, REG_I_1_P1 + offset, REG_I_1_P0 + offset, 
			REG_I_1_K + offset, REG_I_1_B + offset);
}
//------------------------------------

Int16U MEASURE_ConvertUset(float Uset)
{
	float Result = Uset * Uset * DataTable[REG_U_SET_P2] + Uset * DataTable[REG_U_SET_P1] + DataTable[REG_U_SET_P0];
	Result = Result * DataTable[REG_U_SET_K] + DataTable[REG_U_SET_B];
	return (Int16U)((Result / DataTable[REG_U_ADC_REF]) * ADC_RESOLUTION);
}
//------------------------------------
