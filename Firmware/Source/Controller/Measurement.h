#ifndef __MEASUREMENT_H
#define __MEASUREMENT_H

#include "stdinc.h"

// Types
typedef enum IChannel
{
	I_CHANNEL_NONE = 0,
	I_CHANNEL_1,
	I_CHANNEL_2,
	I_CHANNEL_3,
	I_CHANNEL_4,
	I_CHANNEL_5,
	I_CHANNEL_6,
	I_CHANNEL_7,
	I_CHANNEL_8
} IChannel;

typedef enum RelayMask
{
	RELAY_NONE = 0,
	RELAY_CH_1 = 0x01,
	RELAY_CH_2 = 0x02,
	RELAY_CH_3 = 0x04,
	RELAY_CH_4 = 0x08,
	RELAY_CH_5 = 0x10,
	RELAY_CH_6 = 0x20,
	RELAY_CH_7 = 0x40,
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
