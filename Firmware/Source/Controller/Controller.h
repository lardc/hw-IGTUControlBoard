#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
//
#include "stdinc.h"
#include "Global.h"
#include "Regulator.h"

// Types
//
typedef enum __DeviceState
{
	DS_None = 0,
	DS_Fault = 1,
	DS_Disabled = 2,
	DS_Ready = 3,
	DS_InProcess = 4,
	DS_Selftest = 5
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None = 0,
	SS_VgsPrepare = 1,
	SS_VgsPulse = 2,
	SS_VgsSaveResults = 3,

	SS_IgesPulse = 3,
	SS_IgesWaitAfterPulse = 4,
	SS_QgWaitAfterPulse = 5,
	SS_QgPulse = 6
} DeviceSubState;


// Variables
//
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern volatile Int64U CONTROL_TimeCounter;
//
extern volatile float CONTROL_RegulatorOutputValues[VALUES_x_SIZE];
extern volatile float CONTROL_VoltageValues[VALUES_x_SIZE];
extern volatile float CONTROL_CurrentValues[VALUES_x_SIZE];
extern volatile float CONTROL_RegulatorErrValues[VALUES_x_SIZE];
extern volatile Int16U CONTROL_RegulatorValues_Counter;
extern volatile Int16U CONTROL_Values_Counter;



extern volatile Int16U CONTROL_TimerMaxCounter;
extern volatile Int64U CONTROL_C_TimeCounter;
extern Int64U CONTROL_LEDTimeout;
extern volatile Int16U CONTROL_V_Values_Counter;
extern volatile Int16U CONTROL_I_Values_Counter;
extern volatile Int16U CONTROL_V_RegErrValues_Counter;
//


// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_ForceStopProcess();
void CONTROL_HighPriorityProcess();
void CONTROL_VGS_SaveResults(volatile RegulatorParamsStruct* Regulator);
void CONTROL_V_StartProcess();
void CONTROL_V_StopProcess();




void CONTROL_IGES_StartProcess();
void CONTROL_QG_StartProcess();
void CONTROL_IGES_SetResults(volatile RegulatorParamsStruct* Regulator);
void CONTROL_QG_SetResults();
void CONTROL_C_StopProcess();
void CONTROL_C_Processing();

#endif // __CONTROLLER_H
