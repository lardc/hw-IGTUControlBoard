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

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, Qi = 0, PrevSetPoint = 0, FollowingErrThreshold, VoltagErrThreshold;
static Int16U Index = 0;
static Int16U FollowingErrLimit, VoltagErrLimit, VoltageErrCount, FollowingErrorCounter = 0;
static float PulseAmplitude = 0;
bool IsVoltageOk = false;

PulseSamples REGLTR_PulseSamples = {0};

// Forward functions
SamplingResult REGLTR_GetSample();
void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error);

// Functions
void REGLTR_Process()
{
	if (CONTROL_SubState != SS_RegulatorProcess)
		return;

	// Получение результата оцифровки и расчёт ошибки
	SamplingResult Sample = REGLTR_GetSample();
	float RegulatorError = PrevSetPoint - Sample.Ug;
	float absError = (RegulatorError >= 0.0f) ? RegulatorError : -RegulatorError;
	if (absError > FollowingErrThreshold)
	{
		if (FollowingErrorCounter < FollowingErrLimit)
			FollowingErrorCounter++;
		else
		{
			CONTROL_SetDeviceSubState(SS_FollowingErr);
			return;
		}
	}
	else
		FollowingErrorCounter = 0;

	// Расчет ошибки по напряжению
	float VoltageErr = ABS(Sample.Ug - PulseAmplitude);
	if(VoltageErr < VoltagErrThreshold)
		IsVoltageOk = true;
	else
	{
		VoltageErrCount++;
		if(VoltageErrCount > VoltagErrLimit)
		{
			CONTROL_SetDeviceSubState(SS_VoltageErr);
			return;
		}
	}

	float Qp = RegulatorError * Kp;
	Qi += RegulatorError * Ki;

	// Расчёт следующего задания и его корректировка
	float RawSetpoint = REGLTR_GetSetpoint(Index);
	float Setpoint = RawSetpoint + Qp + Qi;

	Int16U DACSetpoint = MEASURE_ConvertUset(Setpoint);
	LL_WriteDAC(DACSetpoint);

	PrevSetPoint = Setpoint;

	REGLTR_StoreRegulatorDebug(Sample.Ug, Sample.UPot, Sample.Ig, RawSetpoint, Qp + Qi, RegulatorError);

	if (Index < REGLTR_PulseSamples.TotalSamples)
		Index++;
	else
		Index = REGLTR_PulseSamples.TotalSamples;
}
//-----------------------------------------

void REGLTR_Init()
{
	IsVoltageOk = false;
	Index = Qi = PrevSetPoint = FollowingErrorCounter = VoltageErrCount = 0;
	FollowingErrThreshold = DataTable[REG_RGLTR_FOLLOWING_ERR_THRESH];
	FollowingErrLimit = DataTable[REG_RGLTR_FOLLOWING_ERR_LIMIT];
	VoltagErrThreshold = DataTable[REG_VOLTAGE_ERR_THRESH];
	VoltagErrLimit = DataTable[REG_VOLTAGE_ERR_COUNT_LIMIT];
	switch(CONTROL_MeasureType)
	{
		case MT_Iges:
			PulseAmplitude = DataTable[REG_WORK_VOLTAGE_IGES] * 0.001;
			break;

		case MT_Rth:
			PulseAmplitude = DataTable[REG_WORK_VOLTAGE_RTH] * 0.001;
			break;
	}
	Kp = DataTable[REG_RGLTR_Kp];
	Ki = DataTable[REG_RGLTR_Ki];

	for (Int16U i = 0; i < ADC_SEQ_LENGTH; ++i)
	{
		REGLTR_MemBuffUg[i] = 0;
		REGLTR_MemBuffUPot[i] = 0;
		REGLTR_MemBuffIg[i] = 0;
	}

	float RiseTime = PulseAmplitude / DataTable[REG_SLEW_RATE];
	float PulseTime = RiseTime + DataTable[REG_PULSE_WIDTH] + RiseTime;

	REGLTR_PulseSamples.TotalSamples = (Int16U)(PulseTime * SAMPLE_RATE);

	REGLTR_PulseSamples.RiseSamples = (Int16U)(REGLTR_PulseSamples.TotalSamples * RiseTime / PulseTime + 0.5f);
	REGLTR_PulseSamples.FlatTopSamples = (Int16U)(REGLTR_PulseSamples.TotalSamples * DataTable[REG_PULSE_WIDTH] / PulseTime + 0.5f);

	Int16U sum = REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.FlatTopSamples;
	if (sum > REGLTR_PulseSamples.TotalSamples)
		REGLTR_PulseSamples.FlatTopSamples = REGLTR_PulseSamples.TotalSamples - REGLTR_PulseSamples.RiseSamples;

	REGLTR_PulseSamples.FallSamples = REGLTR_PulseSamples.TotalSamples - REGLTR_PulseSamples.RiseSamples - REGLTR_PulseSamples.FlatTopSamples;
}
//-----------------------------------------

float REGLTR_GetSetpoint(Int16U i)
{
	if (i < REGLTR_PulseSamples.RiseSamples)
		return (float)i / REGLTR_PulseSamples.RiseSamples * PulseAmplitude;
	else if (i < REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.FlatTopSamples)
		return PulseAmplitude;
	else if (i < REGLTR_PulseSamples.TotalSamples)
	{
		Int16U fallIdx = i - (REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.FlatTopSamples);
		return PulseAmplitude * (1.0f - (float)fallIdx / REGLTR_PulseSamples.FallSamples);
	}
	
	return 0;
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
	t.Ig   = MEASURE_I(avgIg, I_CHANNEL_0);
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
	DMA_ChannelEnable(DMA1_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel5, false);
	TIM_Stop(TIM15);
}
//------------------------------------
