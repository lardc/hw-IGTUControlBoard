// Header
#include "Diagnostic.h"

// Include
#include "Delay.h"
#include "LowLevel.h"
#include "Board.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"

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
			GPIO_SetState(GPIO_SPI_OE, false);

			LL_SPI_WriteByte(0b10101010);
			DELAY_MS(500);
			LL_SPI_WriteByte(0b01010101);

			DELAY_US(TIME_SPI_DELAY);
			GPIO_SetState(GPIO_SPI_SS, true);
			DELAY_US(TIME_SPI_DELAY);
			GPIO_SetState(GPIO_SPI_SS, false);
			DELAY_US(TIME_SPI_DELAY);
			GPIO_SetState(GPIO_SPI_OE, true);
			break;

		default:
			return false;
	}

	return true;
}
//-----------------------------
