#ifndef __MEASUREMENT_H
#define __MEASUREMENT_H

#include "stdinc.h"

// Types
typedef enum IChannel
{
	I_CHANNEL_1 = 0,
	I_CHANNEL_2,
	I_CHANNEL_3,
	I_CHANNEL_4,
	I_CHANNEL_5,
	I_CHANNEL_6,
	I_CHANNEL_7,
	I_CHANNEL_8
} IChannel;

// Functions
//
float MEASURE_U1(Int16U SampleADC);
float MEASURE_U2(Int16U SampleADC);
float MEASURE_I(Int16U SampleADC, IChannel Channel);
//
Int16U MEASURE_ConvertUset(float Uset);

#endif // __MEASUREMENT_H
