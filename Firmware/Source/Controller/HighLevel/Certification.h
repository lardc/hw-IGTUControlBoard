#ifndef CERTIFICATION_H_
#define CERTIFICATION_H_

// Include
//
#include "Board.h"
#include "stdinc.h"
#include "Regulator.h"

// Variables
//
extern pFloat32 SertSet_V, SertSet_Vneg, SertSet_I, SertSet_Tp, SertResult_V, SertResult_I, SertResult_Q;
extern RegulatorMode Mode;

// Functions prototypes
//
void SERT_V_Prepare();
void SERT_I_Prepare();
void SERT_V_Process();
void SERT_I_Process();

#endif /* CERTIFICATION_H_ */
