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
	Log->SumA -= Log->RingBufferA[Log->RingCounter];
	Log->SumB -= Log->RingBufferB[Log->RingCounter];
	//
	Log->RingBufferA[Log->RingCounter] = *Log->DataA;
	Log->RingBufferB[Log->RingCounter] = *Log->DataB;
	//
	Log->SumA += Log->RingBufferA[Log->RingCounter];
	Log->SumB += Log->RingBufferB[Log->RingCounter];

	Log->RingCounter++;
	Log->RingCounter &= Log->RingCounterMask;
}
//-----------------------------------------------

MeasureSample LOG_RingBufferGetAverage(RingBuffersParams* Log)
{
	MeasureSample AverageSamples;

	AverageSamples.Voltage = Log->SumA / (Log->RingCounterMask + 1);
	AverageSamples.Current = Log->SumB / (Log->RingCounterMask + 1);

	return AverageSamples;
}
//-----------------------------------------------

void LOG_ClearBuffers(RingBuffersParams* Log)
{
	Log->RingCounter = 0;
	Log->SumA = 0;
	Log->SumB = 0;

	for(int i = 0; i < LOG_RING_BUFFER_SIZE; i++)
	{
		Log->RingBufferA[i] = 0;
		Log->RingBufferB[i] = 0;
	}
}

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
			if(*(Buffer + i) > DataTable[REG_I_ADC_TO_V_ZERO_OFFSET])
				*(Endpoint + Counter) = CU_I_ADCtoV(*(Buffer + i), MEASURE_I_VC);
			else
				*(Endpoint + Counter) = CU_I_ADCtoV(*(Buffer + i), MEASURE_I_VN);
			Counter++;
		}
	}
}
//-----------------------------------------------

void LOG_CopyCurrentToEndpoints(pFloat32 Endpoint, volatile Int16U* Buffer, Int16U BufferSize)
{
	Int16U Counter = 0;

	for(Int16U i = 0; i < BufferSize; i ++)
	{
		*(Endpoint + Counter) = CU_I_ADCtoI(*(Buffer + i));
		Counter++;
	}
}
//-----------------------------------------------
