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
float MEASURE_ConvertX(float SampleADC, Int16U RegisterP2, Int16U RegisterP1, Int16U RegisterP0, Int16U RegisterK, Int16U RegisterB, Int16U RegisterRshunt);

// Functions
/**
 * Convert a block of raw ADC samples in-place to calibrated physical values using register-stored coefficients.
 *
 * Applies per-sample scaling and calibration using the polynomial coefficients P2, P1, P0 and the linear parameters K and B
 * read from the DataTable at the provided register indices. If the Rshunt register value is non-zero, each intermediate
 * result is divided by that shunt resistance before the polynomial is applied. Converted values are clamped to a minimum of 0.
 *
 * @param InputArray Pointer to an array of samples; each element is replaced in-place with its calibrated value.
 * @param DataLength Number of elements in InputArray to convert.
 * @param RegisterP2 Index in DataTable of the quadratic coefficient P2.
 * @param RegisterP1 Index in DataTable of the linear coefficient P1.
 * @param RegisterP0 Index in DataTable of the constant coefficient P0.
 * @param RegisterK  Index in DataTable of the scale coefficient K.
 * @param RegisterB  Index in DataTable of the bias term B.
 * @param RegisterRshunt Index in DataTable of the shunt resistance; if its value is zero, shunt division is skipped.
 */
void MEASURE_ConvertADC_X(pFloat32 InputArray, Int16U DataLength, Int16U RegisterP2, Int16U RegisterP1, Int16U RegisterP0, Int16U RegisterK, Int16U RegisterB, Int16U RegisterRshunt)
{
	Int16U P2 = DataTable[RegisterP2];
	Int16U P1 = DataTable[RegisterP1];
	Int16U P0 = DataTable[RegisterP0];
	Int16U K = DataTable[RegisterK];
	Int16U B = DataTable[RegisterB];
	Int16U Rshunt = DataTable[RegisterRshunt];
	
	for(Int16U i = 0; i < DataLength; i++)
	{
		float Result = (float)(*((pInt32U)(InputArray + i)));
		Result = Result * DataTable[REG_U_ADC_REF] * K + B;

		if(Rshunt)
			Result = Result / Rshunt;

		Result = Result * Result * P2 + Result * P1 + P0;
		Result = (Result > 0) ? Result : 0;

		InputArray[i] = Result;
	}
}
/**
 * Convert an ADC sample to a calibrated physical value using calibration coefficients stored in registers.
 * @param SampleADC ADC sample value (float) to convert.
 * @param RegisterP2 Register index containing the quadratic coefficient (P2).
 * @param RegisterP1 Register index containing the linear coefficient (P1).
 * @param RegisterP0 Register index containing the constant term (P0).
 * @param RegisterK Register index containing the scale factor applied to the ADC voltage.
 * @param RegisterB Register index containing the offset added after scaling.
 * @param RegisterRshunt Register index of the shunt resistance to divide by; pass 0 if no shunt division is required.
 * @return Converted physical value computed from the calibrated polynomial, clamped to be >= 0.
 */

float MEASURE_ConvertX(float SampleADC, Int16U RegisterP2, Int16U RegisterP1, Int16U RegisterP0, Int16U RegisterK, Int16U RegisterB, Int16U RegisterRshunt)
{
	float Result = (SampleADC / ADC_RESOLUTION) * DataTable[REG_U_ADC_REF] * DataTable[RegisterK] + DataTable[RegisterB];

	if (RegisterRshunt)
		Result /= DataTable[RegisterRshunt];

	Result = Result * Result * DataTable[RegisterP2] + Result * DataTable[RegisterP1] + DataTable[RegisterP0];

	return (Result > 0) ? Result : 0;
}
/**
 * Convert an ADC sample to the sensed gas voltage using UG calibration registers.
 * @param SampleADC Raw ADC sample value.
 * @returns Converted gas voltage computed from the UG calibration coefficients.
 */

float MEASURE_Ug(float SampleADC)
{
	return MEASURE_ConvertX(SampleADC, REG_U_G_P2, REG_U_G_P1, REG_U_G_P0, REG_U_G_K, 
			REG_U_G_B, 0);
}
/**
 * Convert an ADC sample to the potentiometer voltage using UPot calibration registers.
 *
 * @returns Converted potentiometer voltage value; negative results are clamped to 0.
 */

float MEASURE_UPot(float SampleADC)
{
	return MEASURE_ConvertX(SampleADC, REG_U_POT_P2, REG_U_POT_P1, REG_U_POT_P0, REG_U_POT_K, 
			REG_U_POT_B, 0);
}
/**
 * Convert an ADC sample to a calibrated current for a specific channel.
 *
 * @param SampleADC ADC sample value (scaled as float) to convert.
 * @param Channel Measurement channel to use for channel-specific calibration coefficients.
 * @returns Calibrated current for the specified channel (floating-point). The value is clamped to be greater than or equal to 0.
 */

float MEASURE_I(float SampleADC, IChannel Channel)
{
	Int16U offset = 6 * (Channel - 1);

	return MEASURE_ConvertX(SampleADC, REG_I_0_P2 + offset, REG_I_0_P1 + offset, REG_I_0_P0 + offset, 
			REG_I_0_K + offset, REG_I_0_B + offset, REG_I_0_RSH + offset);
}
/**
 * Convert a target Uset value into the corresponding ADC register value.
 *
 * @param Uset Target setpoint value (voltage or normalized unit) to convert.
 * @returns ADC register value scaled according to calibration coefficients and ADC reference, clamped to the range 0..ADC_RESOLUTION.
 */

Int16U MEASURE_ConvertUset(float Uset)
{
	float Result = Uset * Uset * DataTable[REG_U_SET_P2] + Uset * DataTable[REG_U_SET_P1] + DataTable[REG_U_SET_P0];
	Result = Result * DataTable[REG_U_SET_K] + DataTable[REG_U_SET_B];
	Result = (Int16U)((Result / DataTable[REG_U_ADC_REF]) * ADC_RESOLUTION);

	if (Result < 0)
		Result = 0;
	else if (Result > ADC_RESOLUTION)
		Result = ADC_RESOLUTION;

	return (Int16U)Result;
}
/**
 * Convert an array of ADC scope samples for a specific current channel into calibrated values in-place.
 *
 * Applies channel-specific calibration (polynomial P2/P1/P0, scale K/B and shunt Rsh) to each element of InputArray
 * and writes the converted physical values back into the same array.
 *
 * @param InputArray Pointer to an array of floats containing ADC samples; values are replaced in-place with calibrated results.
 * @param DataLength Number of elements in InputArray.
 * @param Channel One-based current channel index used to select the channel-specific calibration registers.
 */

void MEASURE_ConvertIScope(pFloat32 InputArray, Int16U DataLength, IChannel Channel)
{
	Int16U offset = 6 * (Channel - 1);

	MEASURE_ConvertADC_X(
		InputArray,
		DataLength,
		REG_I_0_P2 + offset,
		REG_I_0_P1 + offset,
		REG_I_0_P0 + offset,
		REG_I_0_K + offset,
		REG_I_0_B + offset,
		REG_I_0_RSH + offset
	);
}
//------------------------------------