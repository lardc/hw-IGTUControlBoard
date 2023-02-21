#ifndef CONTROLLER_LOGGING_H_
#define CONTROLLER_LOGGING_H_

// Includes
//
#include "stdinc.h"
#include "Measurement.h"


// Definitions
//
#define LOG_RING_BUFFER_SIZE			64
#define LOG_COUNTER_MASK				LOG_RING_BUFFER_SIZE - 1

// Structs
//
typedef struct __LogParams
{
	pFloat32 DataA;
	pFloat32 DataB;
	pFloat32 LogBufferA;
	pFloat32 LogBufferB;
	pInt16U LogBufferCounter;
} LogParamsStruct;

typedef struct __RingBuffersParams
{
	pFloat32 DataA;
	pFloat32 DataB;
	Int16U RingCounter;
	float RingBufferA[LOG_RING_BUFFER_SIZE];
	float RingBufferB[LOG_RING_BUFFER_SIZE];
} RingBuffersParams;

// Functions
//
void LOG_LoggingData(LogParamsStruct* Log);
void LOG_SaveSampleToRingBuffer(RingBuffersParams* Log);
MeasureSample LOG_GetAverage(RingBuffersParams* Log);

#endif /* CONTROLLER_LOGGING_H_ */
