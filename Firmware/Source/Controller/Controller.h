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
	DS_InProcess		= 4,
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None 			= 0,
	SS_Init				= 1,
	SS_Wait48VPause		= 2,
	SS_ConfigPulse		= 3,
	SS_RegulatorProcess = 4,
	SS_FollowingErr		= 5,
	SS_VoltageErr		= 6,
	SS_FinishProcess	= 7,
	SS_GetResults 		= 8,
} DeviceSubState;

typedef enum __MeasureType
{
	MT_Rth = 0,
	MT_Iges = 1,
} MeasureType;

// Variables
extern volatile Int64U CONTROL_TimeCounter;

extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern volatile MeasureType CONTROL_MeasureType;

extern Int16U CONTROL_ExtInfoCounter;
extern Int16U CONTROL_ExtInfoData[];

extern Int16U CONTROL_Values_Counter;
extern float CONTROL_RegulatorIg[];
extern float CONTROL_RegulatorUg[];
extern float CONTROL_RegulatorUpot[];
extern float CONTROL_RegulatorSetpoint[];
extern float CONTROL_RegulatorCorrection[];
extern float CONTROL_RegulatorError[];

// Functions
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SetDeviceState(DeviceState NewState);
void CONTROL_SetDeviceSubState(DeviceSubState NewSubState);
void CONTROL_SwitchToProblem(Int16U Reason);

#endif // __CONTROLLER_H
