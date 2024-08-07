﻿#ifndef __LOWLEVEL_H
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
void LL_QgProtection(bool State);
void LL_ToggleIndication();
void LL_OutMultiplexCurrent();
void LL_OutMultiplexVoltage();
void LL_SyncTOCUHP(bool State);
void LL_SyncOSC(bool State);
void LL_SyncPAU(bool State);
bool LL_SafetyState();
//
void LL_V_ShortOut(bool State);
void LL_V_ShortPAU(bool State);
void LL_V_VSetDAC(Int16U Data);
void LL_V_IlimLowRange();
void LL_V_IlimHighRange();
void LL_V_Diagnostic(bool State);
void LL_V_IsenseHighRange0();
void LL_V_IsenseHighRange1();
//
void LL_I_Start(bool State);
void LL_I_Enable(bool State);
void LL_I_SetDAC(Int16U Data);
void LL_I_Diagnostic(bool State);
void LL_ExtDACSendData(Int16U Data);
void LL_ExDACVCutoff(Int16U Value);
void LL_ExDACVNegative(Int16U Value);

#endif //__LOWLEVEL_H
