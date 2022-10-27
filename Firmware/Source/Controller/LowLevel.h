#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_UShortOut(bool State);
void LL_IStart(bool State);
bool LL_ICompState();
void LL_IISetDAC(Int16U Data);
void LL_UUSetDAC(Int16U Data);
void LL_ExtDACSync(bool State);
void LL_ExtDACLDAC(bool State);
void LL_ExtDACSendData(Int16U Data);

#endif //__LOWLEVEL_H
