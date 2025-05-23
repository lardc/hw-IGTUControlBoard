#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_ExtIndication(bool State);
void LL_ToggleExternalLED();
void LL_SPI_WriteByte(Int8U Data);

#endif //__LOWLEVEL_H
