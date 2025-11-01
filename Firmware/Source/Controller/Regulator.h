#ifndef __REGULATOR_H
#define __REGULATOR_H

// Include
#include "stdinc.h"

#define SAMPLE_RATE (1000000.0f / TIMER15_uS)

typedef struct {
	Int16U RiseSamples;
	Int16U PlateuSamples;
	Int16U FallSamples;
	Int16U TotalSamples;
} PulseSamples;

extern PulseSamples REGLTR_PulseSamples;

extern Int16U REGLTR_MemBuffUg[];
extern Int16U REGLTR_MemBuffUPot[];
extern Int16U REGLTR_MemBuffIg[];

void REGLTR_Process();
float REGLTR_GetSetpoint(Int16U i);

#endif // __REGULATOR_H
