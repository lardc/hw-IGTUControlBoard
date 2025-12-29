// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "ZwSPI.h"

// Variables
uint16_t PrevMask = 0;

// Forward functions
//
void LL_SPI_SetStateOE(bool State);

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

void LL_SPI_WriteByte(Int16U Data)
{
	SPI_WriteByte(SPI1, Data);
}
//-----------------------------

void LL_SPI_SetStateOE(bool State)
{
	GPIO_SetState(GPIO_SPI_OE, !State);
}

void LL_WriteDAC(Int16U Data)
{
	DAC_SetValueCh1(DAC1, Data);
}
//-----------------------------

void LL_SetCurrentChannel(IChannel Channel)
{
	uint16_t Mask = PrevMask;
	LL_SPI_SetStateOE(true);
	switch(Channel)
	{
		case I_CHANNEL_0:
			Mask &=~ (RELAY_CH_1|RELAY_CH_2|RELAY_CH_3|RELAY_CH_4|RELAY_CH_5|RELAY_CH_6|RELAY_CH_7);
			Mask |= RELAY_CH_0;
			LL_SPI_WriteByte(Mask);
			PrevMask = Mask;
			break;
		case I_CHANNEL_1:
			LL_SPI_WriteByte(RELAY_CH_1);
			break;
		case I_CHANNEL_2:
			LL_SPI_WriteByte(RELAY_CH_2);
			break;
		case I_CHANNEL_3:
			LL_SPI_WriteByte(RELAY_CH_3);
			break;
		case I_CHANNEL_4:
			LL_SPI_WriteByte(RELAY_CH_4);
			break;
		case I_CHANNEL_5:
			LL_SPI_WriteByte(RELAY_CH_5);
			break;
		case I_CHANNEL_6:
			LL_SPI_WriteByte(RELAY_CH_6);
			break;
		case I_CHANNEL_7:
			LL_SPI_WriteByte(RELAY_CH_7);
			break;
	}
	LL_SPI_SetStateOE(false);
}
//-----------------------------

bool LL_SafetyState()
{
	return GPIO_GetState(GPIO_SAFETY);
}
//-----------------------------
