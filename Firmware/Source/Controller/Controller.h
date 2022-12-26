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
	SS_VgsPulse = 2,
	SS_VgsWaitAfterPulse = 3,
	SS_IgesPulse = 4,
	SS_IgesWaitAfterPulse = 5,
	SS_QgWaitAfterPulse = 6,
	SS_QgPulse = 7,
	SS_SelfTest = 8
} DeviceSubState;


// Variables
//
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
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
void CONTROL_DelayMs(uint32_t Delay);

void CONTROL_V_HighPriorityProcess();
void CONTROL_C_HighPriorityProcess();

void CONTROL_VGS_StartProcess();
void CONTROL_IGES_StartProcess();
void CONTROL_V_StartProcess();

void CONTROL_QG_StartProcess();

void CONTROL_VGS_SetResults(volatile RegulatorParamsStruct* Regulator);
void CONTROL_IGES_SetResults(volatile RegulatorParamsStruct* Regulator);
void CONTROL_QG_SetResults();

void CONTROL_V_StopProcess();

void CONTROL_C_StopProcess();
void CONTROL_C_Processing();

#endif // __CONTROLLER_H
