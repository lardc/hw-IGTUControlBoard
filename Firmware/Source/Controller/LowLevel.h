#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"

#define LL_V_LOW_CURRENT_LIMIT		0	// 1 mA limit
#define LL_V_HIGH_CURRENT_LIMIT 	1	// 200 mA limit

#define LL_V_LOW_CURRENT_SENS_COEF	0	// 0 - 20 mA sensing
#define LL_V_HIGH_CURRENT_SENS_COEF	1	// 20 - 200 mA sensing

// Functions
//
void LL_ToggleBoardLED();
void LL_Indication(bool State);
void LL_OutMultiplexCurrent();
void LL_OutMultiplexVoltage();
void LL_OutMultiplex(bool State);
void LL_SyncTOCUHP(bool State);
void LL_SyncOSC(bool State);
void LL_SyncPAU(bool State);
bool LL_FeedbackPAU();
bool LL_SafetyState();
//
void LL_V_ShortOut(bool State);
void LL_V_ShortPAU(bool State);
void LL_V_VSetDAC(Int16U Data);
void LL_V_CoefCSensLowRange();
void LL_V_CoefCSensHighRange();
void LL_V_CLimitLowRange();
void LL_V_CLimitHighRange();
void LL_V_Diagnostic(bool State);
//
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
