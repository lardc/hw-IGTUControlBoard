// Header
//
#include "Logging.h"
#include "ConvertUtils.h"

// Includes
//
#include "DataTable.h"

// Functions
//
void LOG_LoggingData(LogParamsStruct* Log)
{
	// —брос локального счетчика в начале логгировани€
	if(*Log->LogBufferCounter == 0)
		Log->LocalCounter = 0;

	if(Log->ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		Log->ScopeLogStep = 0;

		*(Log->LogBufferA + Log->LocalCounter) = *Log->DataA;
		*(Log->LogBufferB + Log->LocalCounter) = *Log->DataB;
		*Log->LogBufferCounter = Log->LocalCounter;

		Log->LocalCounter++;
	}

	// ”словие обновлени€ глобального счетчика данных
	if(*Log->LogBufferCounter < VALUES_x_SIZE)
		*Log->LogBufferCounter = Log->LocalCounter;

	// —брос локального счетчика
	if(Log->LocalCounter >= VALUES_x_SIZE)
		Log->LocalCounter = 0;
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

MeasureSample LOG_RingBufferGetAverage(RingBuffersParams* Log)
{
	MeasureSample AverageSamples;

	AverageSamples.Voltage = LOG_GetAverageFromBuffer(&Log->RingBufferA[0], LOG_RING_BUFFER_SIZE);
	AverageSamples.Current = LOG_GetAverageFromBuffer(&Log->RingBufferB[0], LOG_RING_BUFFER_SIZE);

	return AverageSamples;
}
//-----------------------------------------------

float LOG_GetAverageFromBuffer(pFloat32 Buffer, Int16U BufferSize)
{
	float Result = 0;

	for(Int16U i = 0; i < BufferSize; i++)
		Result += *(Buffer + i) / BufferSize;

	return Result;
}
//-----------------------------------------------

void LOG_CopyVoltageToEndpoints(pFloat32 Endpoint, volatile Int16U* Buffer, Int16U BufferSize, Int16U SkipStep)
{
	Int16U Counter = 0;

	for(Int16U i = 0; i < BufferSize; i += (SkipStep + 1))
	{
		if(Counter < VALUES_x_SIZE)
		{
			*(Endpoint + Counter) = CU_I_ADCtoV(*(Buffer + i));
			Counter++;
		}
	}
}
//-----------------------------------------------

void LOG_CopyCurrentToEndpoints(pFloat32 Endpoint, volatile Int16U* Buffer, Int16U BufferSize, Int16U SkipStep)
{
	Int16U Counter = 0;

	for(Int16U i = 0; i < BufferSize; i += (SkipStep + 1))
	{
		if(Counter < VALUES_x_SIZE)
		{
			*(Endpoint + Counter) = CU_I_ADCtoI(*(Buffer + i));
			Counter++;
		}
	}
}
//-----------------------------------------------
