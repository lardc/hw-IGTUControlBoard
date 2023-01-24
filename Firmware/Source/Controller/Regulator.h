#ifndef REGULATOR_H_
#define REGULATOR_H_

// Includes
//
#include "stdinc.h"
#include "Global.h"
#include "SysConfig.h"

// Definitions
//
#define STEP_BUFFER_SIZE	V_VALUES_x_SIZE
#define STEP_PERIOD			TIMER15_uS

// Structs
//
typedef struct __RegulatorParams
{
	Int16U VFormTable[STEP_BUFFER_SIZE];
	Int16U VSenForm [STEP_BUFFER_SIZE];
	float VSen;
	float CSen;
	float CTrigVSen;
	float CTrigCSen;
	Int16U CTrigRegulatorStep;
	float Kp;
	float Ki;
	float RegulatorError;
	float RegulatorOutput;
	bool DebugMode;
	Int16U RegulatorStepCounter;
	Int16U ConstantVLastStep;
	Int16U ConstantVFirstStep;
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
bool REGULATOR_IGES_SyncPAU(volatile RegulatorParamsStruct* Regulator);
//

#endif /* REGULATOR_H_ */
