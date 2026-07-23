#ifndef CONTROLLER_VGS_H_
#define CONTROLLER_VGS_H_

// Include
//
#include "Board.h"
#include "stdinc.h"

// Types
//
typedef enum __VgsState
{
	Vgs_None = 0,
	Vgs_Rise = 1,
	Vgs_FlatTop = 2,
	Vgs_Finish = 3,
} VgsState;

// Functions
//
void VGS_Prepare();
void VGS_Process();

#endif /* CONTROLLER_VGS_H_ */
