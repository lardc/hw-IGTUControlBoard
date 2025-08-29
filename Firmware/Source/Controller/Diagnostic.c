// Header
#include "Diagnostic.h"

// Include
#include "Delay.h"
#include "LowLevel.h"
#include "Board.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"
#include "Measurement.h"

// Variables
//
Int16U DIAG_PulseDataBuffer[DIAG_PULSE_BUFFER_SIZE];

// Functions
//
bool DIAG_HandleDiagnosticAction(Int16U ActionID, Int16U *pUserError)
{
	switch (ActionID)
	{
		case ACT_DBG_EXT_INDICATION:
			LL_ExtIndication(true);
			DELAY_MS(500);
			LL_ExtIndication(false);
			break;

		case ACT_DBG_SPI_WRITE_TWO_BYTES:
			LL_SPI_WriteByte(DataTable[REG_DBG]);
			break;

		case ACT_DBG_PULSE:
			DIAG_GenerateTrapezoidWave();
			break;

		default:
			return false;
	}

	return true;
}
//------------------------------------------------

void DIAG_GenerateTrapezoidWave()
{
	float RiseTime = DataTable[REG_PULSE_AMPLITUDE] / DataTable[REG_SLEW_RATE];

	float PulseTime = RiseTime + DataTable[REG_PULSE_WIDTH] + RiseTime;

	Int16U RiseSamples = (Int16U)(DIAG_PULSE_BUFFER_SIZE * RiseTime / PulseTime);
	Int16U PlateuSamples = (Int16U)(DIAG_PULSE_BUFFER_SIZE * DataTable[REG_PULSE_WIDTH] / PulseTime);
	Int16U FallSamples = DIAG_PULSE_BUFFER_SIZE - RiseSamples - PlateuSamples;

	Int16U idx = 0;
	for (Int16U i = 0; i < RiseSamples; ++i)
	{
		float value = (float)i / RiseSamples * DataTable[REG_PULSE_AMPLITUDE];
		DIAG_PulseDataBuffer[idx++] = MEASURE_ConvertUset(value);
	}

	for (Int16U i = 0; i < PlateuSamples; ++i)
		DIAG_PulseDataBuffer[idx++] = MEASURE_ConvertUset(DataTable[REG_PULSE_AMPLITUDE]);

	for (Int16U i = 0; i < FallSamples; ++i)
	{
		float value = DataTable[REG_PULSE_AMPLITUDE] * (1.0f - (float)i / FallSamples);
		DIAG_PulseDataBuffer[idx++] = MEASURE_ConvertUset(value);
	}
}
//------------------------------------------------




