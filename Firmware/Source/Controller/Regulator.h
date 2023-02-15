#ifndef REGULATOR_H_
#define REGULATOR_H_

// Includes
//
#include "stdinc.h"
#include "Global.h"
#include "SysConfig.h"

// Definitions
//
#define REGULATOR_RING_BUFFER_SIZE		8
#define RING_COUNTER_MASK				REGULATOR_RING_BUFFER_SIZE - 1

// Structs
//
typedef struct __RegulatorParams
{
	float Kp;
	float Ki;
	float ErrorMax;
	float Qimax;
	Int16U FECounterMax;
	Int16U DACLimitValue;
	Int16U DACOffset;
	bool DebugMode;
//
	Int16S Counter;
	float dVg;
	float Target;
	float SampledData;
//
	Int16U DACSetpoint;
	float Error;
	float Qi;
	float Qp;
	float Out;
	Int16U FECounter;
	bool FollowingError;
	float RingBuffer[REGULATOR_RING_BUFFER_SIZE];
} RegulatorParamsStruct;

// Variables
//
extern volatile RegulatorParamsStruct RegulatorParams;

// Functions
//
bool REGULATOR_Process(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_CacheCommonVariables(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_CacheVgsVariables(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_ResetVariables(volatile RegulatorParamsStruct* Regulator);
//

#endif /* REGULATOR_H_ */
