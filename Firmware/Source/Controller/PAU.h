#ifndef CONTROLLER_PAU_H_
#define CONTROLLER_PAU_H_

// Includes
//
#include "stdinc.h"

// Definitions
//
#define PAU_CHANNEL_LCTU			1
#define PAU_CHANNEL_IGTU			2
//
#define PAU_CONFIG_TIMEOUT			1000
#define PAU_WAIT_READY_TIMEOUT		5000
#define PAU_SYNC_PERIOD_MAX			2000
//
#define PAU_I_RANGE_0				20e-6	// mA
#define PAU_I_RANGE_1				200e-6	// mA
#define PAU_I_RANGE_2				2e-3	// mA

// Types
//
typedef enum __PAUState
{
	PS_None = 0,
	PS_Fault = 1,
	PS_Disabled = 2,
	PS_Ready = 3,
	PS_InProcess = 4,
	PS_ConfigReady = 5
} PAUState;

// Functions
//
bool PAU_UpdateState(Int16U* Register);
bool PAU_Configure(Int16U Channel, float Range, Int16U SamplesNumber);
bool PAU_ClearFault();
bool PAU_ClearWarning();
bool PAU_ReadMeasuredData(float* Data);
void PAU_ShortInput(bool State);

#endif /* CONTROLLER_PAU_H_ */
