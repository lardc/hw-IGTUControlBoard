#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_SetDAC(Int16U Value, bool SecondDAC);
void LL_Pulse(bool State);

#endif //__LOWLEVEL_H
