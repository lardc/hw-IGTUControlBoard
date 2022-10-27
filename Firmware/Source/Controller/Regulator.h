#ifndef REGULATOR_H_
#define REGULATOR_H_

// Includes
//
#include "stdinc.h"
#include "Global.h"
#include "SysConfig.h"

// Definitions
//
#define PULSE_BUFFER_SIZE	U_VALUES_x_SIZE
#define PULSE_PERIOD		TIMER15_uS

// Structs
//
typedef struct __RegulatorParams
{
	float UMeasured;
	float UFormTable[PULSE_BUFFER_SIZE];
	float UFormMeasured[PULSE_BUFFER_SIZE];
	float IFormMeasured[PULSE_BUFFER_SIZE];
	float Kp;
	float Ki;
	float KiTune;
	float RegulatorError;
	bool DebugMode;
	Int16U RegulatorPulseCounter;
	Int16U ConstantULastPulse;
	Int16U ConstantUFirstPulse;
	Int16U ITrigRegulatorPulse;
	float RegulatorOutput;
	Int16U DACOffset;
	Int16U DACLimitValue;
	Int16U DACSetpoint;
}RegulatorParamsStruct;

// Functions
//
bool REGULATOR_Process(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_CashVariables(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_UFormConfig(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_UFormUpdate (volatile RegulatorParamsStruct* Regulator);
//

#endif /* REGULATOR_H_ */
