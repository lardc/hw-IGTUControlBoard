#ifndef CONTROLLER_TOCUHP_H_
#define CONTROLLER_TOCUHP_H_

// Includes
//
#include "stdinc.h"

// Definitions
#define TOCUHP_NUM_MAX				3
#define TOCUHP_BIT_MASK				0x3FF

// Types
//
typedef enum __TOCUHPState
{
	TS_None = 0,
	TS_Fault = 1,
	TS_Disabled = 2,
	TS_Ready = 3,
	TS_InProcess = 4
} TOCUHPState;

// Variables
//
extern Int16U	TOCUHP_State;

// Functions
//
void TOCUHP_UpdateCANid();
bool TOCUHP_IsReady();
bool TOCUHP_InFault();
bool TOCUHP_ClearFault();
bool TOCUHP_PowerEnable();
void TOCUHP_EmulatedState(bool State);
bool TOCUHP_IsEmulatedState();
bool TOCUHP_PowerDisable();
bool TOCUHP_ConfigAnodeVoltage(Int16U Voltage);
bool TOCUHP_ConfigAnodeCurrent(float Current);

#endif /* CONTROLLER_TOCUHP_H_ */
