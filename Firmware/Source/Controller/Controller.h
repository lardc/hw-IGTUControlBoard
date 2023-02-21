﻿#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
//
#include "stdinc.h"
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

	SS_Cal_V_Prepare = 1,
	SS_Cal_V_Process,
	SS_Cal_I_Prepare,
	SS_Cal_I_Process,

	SS_VgsPrepare = 10,
	SS_VgsProcess,

	SS_IgesPrepare = 20,
	SS_IgesProcess,

	SS_QgPrepare = 30,
	SS_QgProcess
} DeviceSubState;

typedef enum __CommutationState
{
	Current = 0,
	Voltage = 1
} CommutationState;
//


// Variables
//
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSubState CONTROL_SubState;
extern volatile Int64U CONTROL_TimeCounter;
//
extern float CONTROL_RegulatorOutputValues[VALUES_x_SIZE];
extern float CONTROL_VoltageValues[VALUES_x_SIZE];
extern float CONTROL_CurrentValues[VALUES_x_SIZE];
extern float CONTROL_RegulatorErrValues[VALUES_x_SIZE];
extern Int16U CONTROL_RegulatorValues_Counter;
extern Int16U CONTROL_Values_Counter;
//


// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState);
void CONTROL_HighPriorityProcess();
void CONTROL_StartHighPriorityProcesses();
void CONTROL_StopHighPriorityProcesses();
void CONTROL_SwitchOutMUX(CommutationState Commutation);
void CONTROL_SwitchToFault(Int16U Reason);

#endif // __CONTROLLER_H
