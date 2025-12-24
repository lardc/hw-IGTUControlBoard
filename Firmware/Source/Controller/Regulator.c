// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"
#include "Measurement.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Regulator.h"
#include "LowLevel.h"
#include "Board.h"
#include "Utils.h"
#include "Logic.h"

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, KiI, KpI, Qi = 0, FollowingErrThreshold, VoltagErrThreshold, CurrentErrThreshold;
static Int16U FollowingErrLimit, VoltagErrLimit, VoltageErrCount, CurrentErrLimit,CurrentErrCount , FollowingErrorCounter = 0;
static float PulseAmplitude, DesiredCurrent;
float RawSetPoint = 0;
float VoltStep = 0, Qp = 0;
float RegulatorError = 0;
RegulatorState RegState = RS_None;
bool IsMeasureOk = false;

SamplingResult Sample = {0};

// Forward functions
SamplingResult REGLTR_GetSample();
void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error);
float REGLTR_CorrectionAndLog();
void RGLTR_ErrorCheck();

// Functions
void REGLTR_Process()
{
	if (CONTROL_SubState != SS_RegulatorProcess && CONTROL_SubState != SS_RegulatorProcessUgeth)
		return;

	float Setpoint;
	Int16U DACSetpoint;

	Sample = REGLTR_GetSample();
	switch(RegState)
	{
		case RS_Rise:
			{
				RawSetPoint += VoltStep;
				if(CONTROL_MeasureType == MT_Ugeth)
				{
					if(Sample.Ig >= DesiredCurrent)
						RegState = RS_FlatTopUgeth;
				}
				else
				{
					if(RawSetPoint >= PulseAmplitude)
					{
						RawSetPoint = PulseAmplitude;
						RegState = RS_FlatTop;
					}
				}
				Setpoint = REGLTR_CorrectionAndLog();
				DACSetpoint = MEASURE_ConvertUset(Setpoint);
				LL_WriteDAC(DACSetpoint);
			}
			break;

		case RS_FlatTopUgeth:
		case RS_FlatTop:
		default:
			Setpoint = REGLTR_CorrectionAndLog();
			DACSetpoint = MEASURE_ConvertUset(Setpoint);
			LL_WriteDAC(DACSetpoint);
			break;
	}
}
//-----------------------------------------

void REGLTR_Init()
{
	IsMeasureOk = false;
	Qi = FollowingErrorCounter = VoltageErrCount = CurrentErrCount = 0;
	FollowingErrThreshold = DataTable[REG_RGLTR_FOLLOWING_ERR_THRESH];
	FollowingErrLimit = DataTable[REG_RGLTR_FOLLOWING_ERR_LIMIT];
	VoltagErrThreshold = DataTable[REG_VOLTAGE_ERR_THRESH];
	VoltagErrLimit = DataTable[REG_VOLTAGE_ERR_COUNT_LIMIT];
	CurrentErrThreshold = DataTable[REG_CURRENT_ERR_THRESH];
	CurrentErrLimit = DataTable[REG_CURRENT_ERR_COUNT_LIMIT];
	RawSetPoint = 0;
	VoltStep = DataTable[REG_SLEW_RATE] / TIMER15_uS;
	RegState = RS_Rise;
	RegulatorError = 0;

	switch(CONTROL_MeasureType)
	{
		case MT_Iges:
			PulseAmplitude = DataTable[REG_WORK_VOLTAGE_IGES] * 0.001;
			break;

		case MT_Rth:
			PulseAmplitude = DataTable[REG_WORK_VOLTAGE_RTH] * 0.001;
			break;

		case MT_Ugeth:
			PulseAmplitude = DataTable[REG_MAX_VOLTAGE_UGETH];
			DesiredCurrent = DataTable[REG_WORK_CURRENT_UGETH];
			break;
	}
	Kp = DataTable[REG_RGLTR_Kp];
	Ki = DataTable[REG_RGLTR_Ki];
	KpI = DataTable[REG_CURRENT_RGLTR_Kp];
	KiI = DataTable[REG_CURRENT_RGLTR_Ki];

	for (Int16U i = 0; i < ADC_SEQ_LENGTH; ++i)
	{
		REGLTR_MemBuffUg[i] = 0;
		REGLTR_MemBuffUPot[i] = 0;
		REGLTR_MemBuffIg[i] = 0;
	}
}
//-----------------------------------------

float REGLTR_CorrectionAndLog()
{
	RGLTR_ErrorCheck();

	Qp = RegulatorError * (RegState == RS_FlatTopUgeth ? KpI : Kp);
	Qi += RegulatorError * (RegState == RS_FlatTopUgeth ? KiI : Ki);

	float SetPoint = RawSetPoint + Qp + Qi;

	REGLTR_StoreRegulatorDebug(Sample.Ug, Sample.UPot, Sample.Ig, RawSetPoint, Qp + Qi, RegulatorError);

	return SetPoint;
}
//-----------------------------------------

void RGLTR_ErrorCheck()
{
	float CurrentErr, VoltageErr;
	switch(RegState)
	{
		case RS_FlatTopUgeth:
			{
				RegulatorError = Sample.Ig - DesiredCurrent;
				// Расчет метрологической ошибки по току
				CurrentErr = ABS(RegulatorError);
				if(CurrentErr < CurrentErrThreshold)
				{
					IsMeasureOk = true;
					CurrentErrCount = 0;
				}
				else
				{
					CurrentErrCount++;
					if(CurrentErrCount > CurrentErrLimit)
						CONTROL_SetDeviceSubState(SS_CurrentErr);
				}
			}
			break;

		default:
			{
				RegulatorError = RawSetPoint - Sample.Ug;
				// Расчет ошибки по напряжению
				VoltageErr = ABS(Sample.Ug - PulseAmplitude);
				if(VoltageErr < VoltagErrThreshold)
				{
					IsMeasureOk = true;
					VoltageErrCount = 0;
				}
				else
				{
					VoltageErrCount++;
					if(VoltageErrCount > VoltagErrLimit)
						CONTROL_SetDeviceSubState(SS_VoltageErr);
				}
			}
			break;
	}
	float absError = ABS(RegulatorError);
	if(absError > FollowingErrThreshold)
	{
		if(FollowingErrorCounter < FollowingErrLimit)
			FollowingErrorCounter++;
		else
			CONTROL_SetDeviceSubState(SS_FollowingErr);
	}
	else
		FollowingErrorCounter = 0;
}
//-----------------------------------------
SamplingResult REGLTR_GetSample()
{
	SamplingResult t = {0};
	float sumUg = 0, sumUPot = 0, sumIg = 0;

	for (Int16U i = 0; i < ADC_SEQ_LENGTH; ++i)
	{
		sumUg   += REGLTR_MemBuffUg[i];
		sumUPot += REGLTR_MemBuffUPot[i];
		sumIg   += REGLTR_MemBuffIg[i];
	}

	float avgUg   = (float)sumUg   / ADC_SEQ_LENGTH;
	float avgUPot = (float)sumUPot / ADC_SEQ_LENGTH;
	float avgIg   = (float)sumIg   / ADC_SEQ_LENGTH;

	t.Ug   = MEASURE_Ug(avgUg);
	t.UPot = MEASURE_UPot(avgUPot);
	t.Ig   = MEASURE_I(avgIg, LOGIC_ChannelNumber);
	return t;
}
//-----------------------------------------

void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error)
{
	if (CONTROL_Values_Counter < VALUES_DEBUG_RGLTR_SIZE)
	{
		CONTROL_RegulatorUg[CONTROL_Values_Counter] = Ug;
		CONTROL_RegulatorUpot[CONTROL_Values_Counter] = UPot;
		CONTROL_RegulatorIg[CONTROL_Values_Counter] = Ig;
		CONTROL_RegulatorSetpoint[CONTROL_Values_Counter] = Setpoint;
		CONTROL_RegulatorCorrection[CONTROL_Values_Counter] = Correction;
		CONTROL_RegulatorError[CONTROL_Values_Counter] = Error;
		++CONTROL_Values_Counter;
	}
}
//-----------------------------------------

void REGLTR_StartProcess()
{
	DMA_ChannelEnable(DMA1_Channel1, true);
	DMA_ChannelEnable(DMA2_Channel1, true);
	DMA_ChannelEnable(DMA2_Channel5, true);

	TIM_Start(TIM15);
}
//------------------------------------

void REGLTR_StopProcess()
{
	LL_WriteDAC(0);
	DMA_ChannelEnable(DMA1_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel5, false);
	TIM_Stop(TIM15);
}
//------------------------------------
