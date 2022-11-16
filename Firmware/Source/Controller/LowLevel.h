#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_V_ShortOut(bool State);
void LL_C_CStart(bool State);
void LL_C_CSetDAC(Int16U Data);
void LL_V_VSetDAC(Int16U Data);
void LL_ExtDACSync(bool State);
void LL_ExtDACLDAC(bool State);
void LL_ExtDACSendData(Int16U Data);

#endif //__LOWLEVEL_H
