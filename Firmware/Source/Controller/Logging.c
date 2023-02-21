// Header
//
#include "Logging.h"

// Includes
//
#include "DataTable.h"

// Functions
//
void LOG_LoggingData(LogParamsStruct* Log)
{
	static Int16U ScopeLogStep = 0, LocalCounter = 0;

	// —брос локального счетчика в начале логгировани€
	if(*Log->LogBufferCounter == 0)
		LocalCounter = 0;

	if(ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		ScopeLogStep = 0;

		*(Log->LogBufferA + LocalCounter) = *Log->DataA;
		*(Log->LogBufferB + LocalCounter) = *Log->DataB;
		*Log->LogBufferCounter = LocalCounter;

		LocalCounter++;
	}

	// ”словие обновлени€ глобального счетчика данных
	if(*Log->LogBufferCounter < VALUES_x_SIZE)
		*Log->LogBufferCounter = LocalCounter;

	// —брос локального счетчика
	if(LocalCounter >= VALUES_x_SIZE)
		LocalCounter = 0;
}
//-----------------------------------------------

void LOG_SaveSampleToRingBuffer(RingBuffersParams* Log)
{
	Log->RingCounter++;
	Log->RingCounter &= LOG_COUNTER_MASK;

	Log->RingBufferA[Log->RingCounter] = *Log->DataA;
	Log->RingBufferB[Log->RingCounter] = *Log->DataB;
}
//-----------------------------------------------

MeasureSample LOG_GetAverage(RingBuffersParams* Log)
{
	MeasureSample AverageSamples;

	AverageSamples.Current = 0;
	AverageSamples.Voltage = 0;

	for(Int16U i = 0; i < LOG_RING_BUFFER_SIZE; i++)
	{
		AverageSamples.Voltage += *(Log->RingBufferA + i) / LOG_RING_BUFFER_SIZE;
		AverageSamples.Current += *(Log->RingBufferA + i) / LOG_RING_BUFFER_SIZE;
	}

	return AverageSamples;
}
//-----------------------------------------------
