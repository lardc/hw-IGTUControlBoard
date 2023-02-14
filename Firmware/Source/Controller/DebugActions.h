#ifndef __DEBUGACTIONS_H
#define __DEBUGACTIONS_H

// Include
//
#include "ZwBase.h"

// Functions
//
void DBGACT_SyncPAU();
void DBGACT_SyncTOCUHP();
void DBGACT_SyncOSC();
void DBGACT_BlinkExtIndication();
void DBGACT_SwitchMUX();
void DBGACT_ShortPAU();
void DBGACT_V_ShortOut();
void DBGACT_V_VSet();
void DBGACT_V_TestClimLow();
void DBGACT_V_TestClimHigh();
void DBGACT_C_VCutoffSet();
void DBGACT_C_VNegativeSet();
void DBGACT_C_TestPulse();

#endif //__DEBUGACTIONS_H
