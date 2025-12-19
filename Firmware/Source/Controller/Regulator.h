#ifndef __REGULATOR_H
#define __REGULATOR_H

// IncludРµ
#include "stdinc.h"

#define SAMPLE_RATE (1000000.0f / TIMER15_uS)

typedef struct {
	Int16U RiseSamples;
	Int16U FlatTopSamples;
	Int16U FallSamples;
	Int16U TotalSamples;
} PulseSamples;

typedef struct __SamplingResult
{
	float Ug, UPot, Ig;
} SamplingResult;

extern PulseSamples REGLTR_PulseSamples;
extern bool IsVoltageOk;

extern Int16U REGLTR_MemBuffUg[];
extern Int16U REGLTR_MemBuffUPot[];
extern Int16U REGLTR_MemBuffIg[];


void REGLTR_Process();
void REGLTR_Init();
float REGLTR_GetSetpoint(Int16U i);
void REGLTR_StartProcess();
void REGLTR_StopProcess();
SamplingResult REGLTR_GetSample();

#endif // __REGULATOR_H
