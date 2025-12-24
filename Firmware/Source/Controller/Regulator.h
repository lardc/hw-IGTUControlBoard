#ifndef __REGULATOR_H
#define __REGULATOR_H

// Include
#include "stdinc.h"

typedef enum __RegulatorState
{
	RS_None 			= 0,
	RS_Rise 			= 1,
	RS_FlatTop 			= 2,
	RS_FlatTopUgeth		= 3,
} RegulatorState;

typedef struct __SamplingResult
{
	float Ug, UPot, Ig;
} SamplingResult;

extern SamplingResult Sample;
extern bool IsMeasureOk;

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
