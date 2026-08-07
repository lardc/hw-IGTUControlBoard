// Header
#include "RingBuffer.h"

// Variables
//
static float BufferI[RINGBUF_MAX_SIZE], BufferU[RINGBUF_MAX_SIZE];
static Int16U Index = 0, Count = 0, MaxSize = RINGBUF_MAX_SIZE;
static float SumI = 0.0f, SumU = 0.0f;

// Functions
//
void RINGBUF_Reset(Int16U Size)
{
	Index = Count = 0;
	SumI = SumU = 0.0f;
	MaxSize = (Size < RINGBUF_MAX_SIZE) ? Size : RINGBUF_MAX_SIZE;
	for(Int16U i = 0; i < RINGBUF_MAX_SIZE; ++i)
	{
		BufferI[i] = 0.0f;
		BufferU[i] = 0.0f;
	}
}
//-----------------------------------------

void RINGBUF_AddSample(float I, float U)
{
	if(Count >= MaxSize)
	{
		SumI -= BufferI[Index];
		SumU -= BufferU[Index];
	}
	else
		Count++;

	BufferI[Index] = I;
	BufferU[Index] = U;

	SumI += I;
	SumU += U;

	Index++;
	if(Index >= MaxSize)
		Index = 0;
}
//-----------------------------------------

float RINGBUF_GetAvgI()
{
	return (Count == 0) ? 0.0f : (SumI / Count);
}
//-----------------------------------------

float RINGBUF_GetAvgU()
{
	return (Count == 0) ? 0.0f : (SumU / Count);
}
//-----------------------------------------

Boolean RINGBUF_IsFull()
{
	return Count == MaxSize;
}
//-----------------------------------------
