// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_ExtIndication(bool State)
{
	GPIO_SetState(GPIO_LED_EXT, State);
}
//-----------------------------

void LL_ToggleExternalLED()
{
	GPIO_Toggle(GPIO_LED_EXT);
}
//-----------------------------

void LL_SPI_WriteByte(Int8U Data)
{
	for (int i = 7; i >= 0; i--)
	{
		GPIO_SetState(GPIO_SPI_DAT, (Data >> i) & 0x1);
		DELAY_US(TIME_SPI_DELAY);
		GPIO_SetState(GPIO_SPI_CLK, true);
		DELAY_US(TIME_SPI_DELAY);
		GPIO_SetState(GPIO_SPI_CLK, false);
	}
}
//-----------------------------
