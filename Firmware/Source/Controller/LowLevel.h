#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_SetDAC(Int16U Value);
void LL_StartPulse();
void LL_HandlePulse();

#endif //__LOWLEVEL_H
