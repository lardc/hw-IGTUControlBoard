// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "math.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_SetDAC(Int16U Value, bool SecondDAC)
{
	GPIO_SetState(SecondDAC ? GPIO_SPI_CS2 : GPIO_SPI_CS1, false);
	SPI_WriteByte(SPI1, Value);
	GPIO_SetState(SecondDAC ? GPIO_SPI_CS2 : GPIO_SPI_CS1, true);
	DELAY_US(1);
}
//-----------------------------

void LL_Pulse(bool State)
{
	GPIO_SetState(GPIO_PULSE, State);
}
//-----------------------------
