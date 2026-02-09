// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "ZwSPI.h"

// Variables
uint16_t PrevMask = 0;
uint16_t Mask = 0;
Int32U CycleCounters[COMMUTATION_TABLE_SIZE] = {0};
uint16_t const CommMask[] = {RELAY_CH_0, RELAY_CH_1, RELAY_CH_2, RELAY_CH_3, RELAY_CH_4, RELAY_CH_5, RELAY_CH_6, RELAY_CH_7,
						RELAY_POTENT_ENTRY, RELAY_SELFTEST, RELAY_TEST_LOAD, RELAY_POLARITY};

// Forward functions
//
void LL_SPI_SetStateOE(bool State);
void LL_Counter_Increase();

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

void LL_Counter_Increase()
{
	for(uint8_t i = 0; i < COMMUTATION_TABLE_SIZE; i++)
	{
		if((PrevMask & CommMask[i]) == 0 && (Mask & CommMask[i]) == CommMask[i])
			CycleCounters[i]++;
	}
}
//-----------------------------

void LL_SPI_WriteByte(uint16_t Data)
{
	if(DataTable[REG_CNT_ACTIVE])
		LL_Counter_Increase();

	GPIO_SetState(GPIO_SPI_SS, false);
	SPI_WriteByte(SPI1, Data);
	LL_SPI_SetStateOE(true);
	GPIO_SetState(GPIO_SPI_SS, true);
}
//-----------------------------

void LL_SPI_SetStateOE(bool State)
{
	GPIO_SetState(GPIO_SPI_OE, !State);
}
//-----------------------------

void LL_WriteDAC(Int16U Data)
{
	DAC_SetValueCh2(DAC1, Data);
}
//-----------------------------

void LL_SetCurrentChannel(IChannel Channel)
{
	Mask = PrevMask;
	Mask &=~ RELAY_ALL_CHANNELS;
	switch(Channel)
	{
		case I_CHANNEL_0:
			Mask |= RELAY_CH_0;
			break;
		case I_CHANNEL_1:
			Mask |= RELAY_CH_1;
			break;
		case I_CHANNEL_2:
			Mask |= RELAY_CH_2;
			break;
		case I_CHANNEL_3:
			Mask |= RELAY_CH_3;
			break;
		case I_CHANNEL_4:
			Mask |= RELAY_CH_4;
			break;
		case I_CHANNEL_5:
			Mask |= RELAY_CH_5;
			break;
		case I_CHANNEL_6:
			Mask |= RELAY_CH_6;
			break;
		case I_CHANNEL_7:
			Mask |= RELAY_CH_7;
			break;
	}
	LL_SPI_WriteByte(Mask);
	PrevMask = Mask;
}
//-----------------------------

bool LL_SafetyState()
{
	return GPIO_GetState(GPIO_SAFETY);
}
//-----------------------------

void LL_SetNegativePolarity(bool State)
{
	Mask = PrevMask;
	State ? (Mask |= RELAY_POLARITY) : (Mask &=~ RELAY_POLARITY);
	LL_SPI_WriteByte(Mask);
	PrevMask = Mask;
}
//-----------------------------
