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
	DS_InProcess = 4
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None = 0,
	SS_PulsePrepare = 1,
	SS_Pulse = 2,
	SS_VGS_WaitAfterPulse = 3,
	SS_WaitAfterIPulse = 4,
	SS_SelfTest = 5
} DeviceSubState;

typedef enum __DeviceWarning
{
	DW_None = 0,
	DW_CurrentNotReached = 1
} DeviceWarning;

// Variables
//
extern volatile DeviceState CONTROL_State;
extern volatile Int64U CONTROL_TimeCounter;
extern volatile Int16U CONTROL_TimerMaxCounter;
extern volatile Int64U CONTROL_I_TimeCounter;
extern Int64U CONTROL_LEDTimeout;
extern volatile Int16U CONTROL_V_Values_Counter;
extern volatile Int16U CONTROL_I_Values_Counter;
extern volatile Int16U CONTROL_V_RegErrValues_Counter;
extern volatile Int16U CONTROL_V_VValues[];
extern volatile Int16U CONTROL_V_VSenValues[];
extern volatile Int16U CONTROL_V_RegOutValues[];
extern volatile Int16U CONTROL_V_RegErrValues[];
extern volatile Int16U CONTROL_V_VDACRawValues[];
extern volatile Int16U CONTROL_V_CSenValues[];
extern volatile Int16U CONTROL_IIGateValues[C_VALUES_x_SIZE];
//
extern volatile RegulatorParamsStruct RegulatorParams;


// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_SetDeviceWarning(DeviceWarning NewWarning);
void CONTROL_DelayMs(uint32_t Delay);
void CONTROL_V_HighPriorityProcess();
void CONTROL_C_HighPriorityProcess();
void CONTROL_ExternalInterruptProcess();
void CONTROL_V_StartProcess();
void CONTROL_C_StartProcess();
void CONTROL_V_SetResults(volatile RegulatorParamsStruct* Regulator);
void CONTROL_C_SetResults();
void CONTROL_V_StopProcess();
void CONTROL_C_Processing();

#endif // __CONTROLLER_H
