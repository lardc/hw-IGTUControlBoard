#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "stdinc.h"

// Types
typedef enum __DeviceState
{
	DS_None 			= 0,
	DS_Fault 			= 1,
	DS_Disabled 		= 2,
	DS_Ready 			= 3,
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None 			= 0,
} DeviceSubState;

// Variables
extern volatile Int64U CONTROL_TimeCounter;

extern Int16U CONTROL_ExtInfoCounter;
extern Int16U CONTROL_ExtInfoData[];

extern Int16U CONTROL_RegulatorIgCounter;
extern float CONTROL_RegulatorIg[];

extern Int16U CONTROL_RegulatorUgCounter;
extern float CONTROL_RegulatorUg[];

extern Int16U CONTROL_RegulatorUpotCounter;
extern float CONTROL_RegulatorUpot[];

extern Int16U CONTROL_RegulatorSetpointCounter;
extern float CONTROL_RegulatorSetpoint[];

extern Int16U CONTROL_RegulatorCorrectionCounter;
extern float CONTROL_RegulatorCorrection[];

extern Int16U CONTROL_RegulatorErrorCounter;
extern float CONTROL_RegulatorError[];

// Functions
void CONTROL_Init();
void CONTROL_Idle();
DeviceSubState CONTROL_GetSubState();
void CONTROL_SetDeviceState(DeviceState NewState);
void CONTROL_SetDeviceSubState(DeviceSubState NewSubState);
void CONTROL_SwitchToProblem(Int16U Reason);

#endif // __CONTROLLER_H
