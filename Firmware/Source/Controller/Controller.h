#ifndef __CONTROLLER_H
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
	DS_SelfTest = 5
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None = 0,
	SS_PowerOnProcess,

	SS_Cal_V_Prepare = 10,
	SS_Cal_V_Process,
	SS_Cal_I_Prepare,
	SS_Cal_I_Process,

	SS_VgsPrepare = 20,
	SS_VgsProcess,

	SS_IgesPrepare = 30,
	SS_IgesProcess,
	SS_IgesSaveResult,

	SS_QgPrepare = 40,
	SS_QgProcess,
	SS_QgSaveResult,

	SS_V_Prepare_Voltage = 50,
	SS_V_Prepare_Irange0,
	SS_V_Prepare_Irange1,
	SS_V_Prepare_Irange2,
	SS_V_Check,

	SS_I_PrepareStage0 = 60,
	SS_I_PrepareStage1,
	SS_I_Check
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
extern float CONTROL_RegulatorOutputValues[VALUES_x_SIZE1];
extern float CONTROL_VoltageValues[VALUES_x_SIZE2];
extern float CONTROL_CurrentValues[VALUES_x_SIZE2];
extern float CONTROL_RegulatorErrValues[VALUES_x_SIZE1];
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
bool CONTROL_IsSafetyOk();
void CONTROL_ResetHardwareToDefaultState();

#endif // __CONTROLLER_H
