#ifndef __MEASUREMENT_H
#define __MEASUREMENT_H

#include "stdinc.h"

// Types
typedef enum IChannel
{
	I_CHANNEL_NONE = 0,
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
	RELAY_NONE = 0,
	RELAY_CH_0 = BIT0,
	RELAY_CH_1 = BIT0 | BIT4,
	RELAY_CH_2 = BIT0 | BIT5,
	RELAY_CH_3 = BIT0 | BIT6,
	RELAY_CH_4 = BIT0 | BIT7,
	RELAY_CH_5 = BIT0 | BIT3,
	RELAY_CH_6 = BIT0 | BIT2,
	RELAY_CH_7 = BIT0 | BIT1,
} RelayMask;

// Functions
//
float MEASURE_U1(Int16U SampleADC);
float MEASURE_U2(Int16U SampleADC);
float MEASURE_I(Int16U SampleADC, IChannel Channel);
//
void MEASURE_ConvertIScope(pFloat32 InputArray, Int16U DataLength, IChannel Channel);
//
Int16U MEASURE_ConvertUset(float Uset);

#endif // __MEASUREMENT_H
