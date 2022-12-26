#ifndef REGULATOR_H_
#define REGULATOR_H_

// Includes
//
#include "stdinc.h"
#include "Global.h"
#include "SysConfig.h"

// Definitions
//
#define PULSE_BUFFER_SIZE	V_VALUES_x_SIZE
#define PULSE_PERIOD		TIMER15_uS

// Structs
//
typedef struct __RegulatorParams
{
	float VSen;
	float VFormTable[PULSE_BUFFER_SIZE];
	float VSenForm[PULSE_BUFFER_SIZE];
	float CSenForm[PULSE_BUFFER_SIZE];
	float Kp;
	float Ki;
	float RegulatorError;
	bool DebugMode;
	Int16U RegulatorStepCounter;
	Int16U ConstantVLastStep;
	Int16U ConstantVFirstStep;
	Int16U CTrigRegulatorStep;
	float RegulatorOutput;
	Int16U DACOffset;
	Int16U DACLimitValue;
	Int16U DACSetpoint;
} RegulatorParamsStruct;

// Functions
//
bool REGULATOR_Process(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_CashVariables(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_VGS_FormConfig(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_VGS_FormUpdate(volatile RegulatorParamsStruct* Regulator);
void REGULATOR_IGES_FormConfig(volatile RegulatorParamsStruct* Regulator);
//

#endif /* REGULATOR_H_ */
