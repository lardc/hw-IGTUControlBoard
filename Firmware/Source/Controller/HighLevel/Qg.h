// Include
//
#include "Board.h"
#include "stdinc.h"

#ifndef CONTROLLER_HIGHLEVEL_QG_H_
#define CONTROLLER_HIGHLEVEL_QG_H_

// Types
//
typedef enum __QGStages
{
	QG_PreMeasurement = 0,
	QG_MainMeasurement,
	QG_Sertification,
	QG_SelfTest
}QGStages;

// Variables
//
extern QGStages QGMeasureStage;

// Function prototypes
//
void QG_Prepare();
void QG_Pulse(bool State);
void QG_SaveResult();
void QG_ResetConfigStageToDefault();
float QG_CalculateGateCharge(pFloat32 Buffer, Int16U BufferSize);
void QG_SetPulseWidth(float Period);
float QG_ExtractAverageCurrent(pFloat32 Buffer, Int16U BufferSize, Int16U AverageLength);
void QG_RemoveDC(pFloat32 InputArray, Int16U ArraySize);
void QG_Filter(pFloat32 InputArray, Int16U ArraySize);
void QG_CacheVariables();

#endif /* CONTROLLER_HIGHLEVEL_QG_H_ */
