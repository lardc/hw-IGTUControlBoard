#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_Indication(bool State);
void LL_OutMultiplex(bool State);
void LL_SyncTOCUHP(bool State);
void LL_SyncOSC(bool State);
void LL_SyncPAU(bool State);
bool LL_FeedbackPAU();
void LL_V_ShortOut(bool State);
void LL_V_ShortPAU(bool State);
void LL_V_VSetDAC(Int16U Data);
void LL_V_CoefCSens(Int16U Data);
void LL_V_CLimit(Int16U Data);
void LL_V_Diagnostic(bool State);
void LL_C_CStart(bool State);
void LL_C_CEnable(bool State);
void LL_C_CSetDAC(Int16U Data);
void LL_C_Diagnostic(bool State);
void LL_ExtDACSync(bool State);
void LL_ExtDACLDAC(bool State);
void LL_ExtDACSendData(Int16U Data);
void LL_ExDACVCutoff(float Value);
void LL_ExDACVNegative(float Value);

#endif //__LOWLEVEL_H
