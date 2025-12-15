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

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, Qi = 0, PrevSetPoint = 0;
static Int16U Index = 0;
static Int16U FollowingErrorCounter = 0;
static float PulseAmplutide = 0;

PulseSamples REGLTR_PulseSamples = {0};

// Forward functions
float REGLTR_GetSetpoint(Int16U i);
SamplingResult REGLTR_GetSample();
void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error);

// Functions
void REGLTR_Process()
{
	// Получение результата оцифровки и расчёт ошибки
	SamplingResult Sample = REGLTR_GetSample();
	float RegulatorError = PrevSetPoint - Sample.Ug;

	float absError = (RegulatorError >= 0.0f) ? RegulatorError : -RegulatorError;
	if (absError > (float)DataTable[REG_RGLTR_FOLLOWING_ERR_THRESH])
	{
		if (FollowingErrorCounter < DataTable[REG_RGLTR_FOLLOWING_ERR_LIMIT])
			FollowingErrorCounter++;
		else
		{
			CONTROL_SwitchToProblem(PROBLEM_FOLLOWING_ERROR);
			GPIO_SetState(GPIO_VCC_48, false);
			REGLTR_StopProcess();
			LL_SetCurrentChannel(I_CHANNEL_7);
		}
	}
	else
		FollowingErrorCounter = 0;

	float Qp = RegulatorError * Kp;
	Qi += RegulatorError * Ki;

	// Расчёт следующего задания и его корректировка
	float Setpoint = REGLTR_GetSetpoint(Index);
	Setpoint += Qp + Qi;

	Int16U DACSetpoint = MEASURE_ConvertUset(Setpoint);
	LL_WriteDAC(DACSetpoint);

	PrevSetPoint = Setpoint;

	REGLTR_StoreRegulatorDebug(Sample.Ug, Sample.UPot, Sample.Ig, Setpoint, Qp + Qi, RegulatorError);

	if (Index < REGLTR_PulseSamples.TotalSamples)
		Index++;
	else
		Index = REGLTR_PulseSamples.TotalSamples;
}
//-----------------------------------------

void REGLTR_Init()
{
	Index = Qi = PrevSetPoint = FollowingErrorCounter = 0;
	PulseAmplutide = (CONTROL_MeasureType ? DataTable[REG_WORK_VOLTAGE_IGES] : DataTable[REG_WORK_VOLTAGE_RTH])* 0.001;

	Kp = DataTable[REG_RGLTR_Kp];
	Ki = DataTable[REG_RGLTR_Ki];

	for (Int16U i = 0; i < ADC_SEQ_LENGTH; ++i)
	{
		REGLTR_MemBuffUg[i] = 0;
		REGLTR_MemBuffUPot[i] = 0;
		REGLTR_MemBuffIg[i] = 0;
	}

	float RiseTime = PulseAmplutide / DataTable[REG_SLEW_RATE];
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
		return (float)i / REGLTR_PulseSamples.RiseSamples * PulseAmplutide;
	else if (i < REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.FlatTopSamples)
		return PulseAmplutide;
	else if (i < REGLTR_PulseSamples.TotalSamples)
	{
		Int16U fallIdx = i - (REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.FlatTopSamples);
		return PulseAmplutide * (1.0f - (float)fallIdx / REGLTR_PulseSamples.FallSamples);
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
	TIM_Start(TIM15);

	DMA_ChannelEnable(DMA1_Channel1, true);
	DMA_ChannelEnable(DMA2_Channel1, true);
	DMA_ChannelEnable(DMA2_Channel5, true);
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
