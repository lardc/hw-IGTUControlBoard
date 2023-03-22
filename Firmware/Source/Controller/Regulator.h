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
typedef enum __RegulatorMode
{
	FeedBack = 0,
	Parametric = 1,
} RegulatorMode;

typedef struct __RegulatorParams
{
	float Kp;
	float Ki;
	float ErrorMax;
	float Qimax;
	Int16U FECounterMax;
	Int16U DACLimitValue;
	RegulatorMode Mode;
//
	Int64U Counter;
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
extern RegulatorParamsStruct RegulatorParams;

// Functions
//
bool REGULATOR_Process(RegulatorParamsStruct* Regulator);
void REGULATOR_CacheVariables(RegulatorParamsStruct* Regulator);
void REGULATOR_ResetVariables(RegulatorParamsStruct* Regulator);
void REGULATOR_Mode(RegulatorParamsStruct* Regulator, RegulatorMode Mode);
//

#endif /* REGULATOR_H_ */
