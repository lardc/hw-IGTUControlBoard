#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "stdinc.h"

// Types
typedef enum IChannel
{
	I_CHANNEL_0,
	I_CHANNEL_1,
	I_CHANNEL_2,
	I_CHANNEL_3,
	I_CHANNEL_4,
	I_CHANNEL_5,
	I_CHANNEL_6,
	I_CHANNEL_7
} IChannel;

typedef enum RelayMask
{
	RELAY_CH_0 			= 0,
	RELAY_CH_1 			= BIT0 | BIT4,
	RELAY_CH_2 			= BIT0 | BIT5,
	RELAY_CH_3 			= BIT0 | BIT6,
	RELAY_CH_4 			= BIT0 | BIT7,
	RELAY_CH_5 			= BIT0 | BIT3,
	RELAY_CH_6 			= BIT0 | BIT2,
	RELAY_CH_7 			= BIT0 | BIT1,
	RELAY_POTENT_ENTRY	= BIT8,
	RELAY_SELFTEST		= BIT8 | BIT9,
	RELAY_TEST_LOAD		= BIT8 | BIT10,
	RELAY_POLARITY		= BIT8 | BIT11,
} RelayMask;

// Variables
//

// Functions
//
void LL_ToggleBoardLED();
void LL_ExtIndication(bool State);
void LL_ToggleExternalLED();
void LL_SPI_WriteByte(Int16U Data);
void LL_SPI_SetStateOE(bool State);
void LL_WriteDAC(Int16U Data);
void LL_SetCurrentChannel(IChannel Channel);
bool LL_SafetyState();
void LL_SetNegativePolarity(bool State);

#endif //__LOWLEVEL_H
