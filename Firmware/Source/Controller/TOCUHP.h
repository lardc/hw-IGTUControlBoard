#ifndef CONTROLLER_TOCUHP_H_
#define CONTROLLER_TOCUHP_H_

// Includes
//
#include "stdinc.h"

// Definitions
#define TOCUHP_BIT_MASK				0x3FF
#define TOCUHP_VOLTAGE				1500

// Types
//
typedef enum __TOCUHPState
{
	TOCUHP_DS_None 				= 0,
	TOCUHP_DS_Fault 			= 1,
	TOCUHP_DS_Disabled 			= 2,
	TOCUHP_DS_Ready 			= 3,
	TOCUHP_DS_InProcess 		= 4
} TOCUHPState;

// Functions
//
bool TOCUHP_ReadState(Int16U* Register);
bool TOCUHP_Configure(Int16U Voltage, float AnodeCurrent);
bool TOCUHP_CallCommand(uint16_t Command);
bool TOCUHP_AreInStateX(uint16_t State);
bool TOCUHP_IsInFaultOrDisabled();
Int16U TOCUHP_BitMask(float AnodeCurrent);

#endif /* CONTROLLER_TOCUHP_H_ */
