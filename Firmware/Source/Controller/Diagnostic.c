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
#include "Regulator.h"
#include "SysConfig.h"

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
/**
 * Output a trapezoid waveform to the DAC using regulator setpoints.
 *
 * Writes successive DAC values derived from regulator pulse setpoints (REGLTR_GetSetpoint)
 * converted to unsigned voltages and applies the configured inter-sample delay (TIMER15_uS).
 * The number of samples emitted is taken from REGLTR_PulseSamples.TotalSamples.
 */

void DIAG_GenerateTrapezoidWave()
{
	for (Int16U i = 0; i < REGLTR_PulseSamples.TotalSamples; ++i)
	{
		LL_WriteDAC(MEASURE_ConvertUset(REGLTR_GetSetpoint(i)));
		DELAY_US(TIMER15_uS);
	}
}
//------------------------------------------------

