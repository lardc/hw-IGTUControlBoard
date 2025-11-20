// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "ZwSPI.h"

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
/**
 * Write a byte to the SPI1 peripheral.
 * @param Data Byte value to transmit over SPI1.
 */

void LL_SPI_WriteByte(Int16U Data)
{
	SPI_WriteByte(SPI1, Data);
}
/**
 * Set DAC channel 1 output value.
 * @param Data Value to load into DAC channel 1.
 */

void LL_WriteDAC(Int16U Data)
{
	DAC_SetValueCh1(DAC1, Data);
}
//-----------------------------