// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"
#include "Measurement.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Regulator.h"
#include "LowLevel.h"

// Definitions
typedef struct __SamplingResult
{
	float Ug, UPot, Ig;
} SamplingResult;

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, Qi = 0, PrevSetPoint = 0;
static Int16U Index = 0;
static Int16U FollowingErrorCounter = 0;

PulseSamples REGLTR_PulseSamples = {0};

// Forward functions
float REGLTR_GetSetpoint(Int16U i);
SamplingResult REGLTR_GetSample();

// Functions
void REGLTR_Process()
{
	// Получение результата оцифровки и расчёт ошибки
	SamplingResult Sample = REGLTR_GetSample();
	float RegulatorError = PrevSetPoint - Sample.Ug;

	if (FollowingErrorCounter > DataTable[REG_RGLTR_FOLLOWING_ERR_THRESH])
	{
		FollowingErrorCounter++;

		if (FollowingErrorCounter > DataTable[REG_RGLTR_FOLLOWING_ERR_LIMIT])
		{
			// заглушка
		}
	}
	else
		FollowingErrorCounter = 0;

	float Qp = RegulatorError * Kp;
	Qi += RegulatorError * Ki;

	// Расчёт следующего задания и его корректировка
	float Sepoint = REGLTR_GetSetpoint(Index);
	Sepoint += Qp + Qi;

	Int16U DACSetpoint = MEASURE_ConvertUset(Sepoint);
	LL_WriteDAC(DACSetpoint);

	// Вызов функции записи в EP отладочной информации
	// 1, 2, 3 оцифрованные значения
	// 4 - задание
	// 5 - скорректированное задание
	// 6 - ошибка

	Index++;
}
//-----------------------------------------

void REGLTR_Init()
{
	Index = Qi = PrevSetPoint = FollowingErrorCounter = 0;

	Kp = DataTable[REG_RGLTR_Kp];
	Ki = DataTable[REG_RGLTR_Ki];

	for (Int16U i = 0; i < ADC_SEQ_LENGTH; ++i)
	{
		REGLTR_MemBuffUg[i] = 0;
		REGLTR_MemBuffUPot[i] = 0;
		REGLTR_MemBuffIg[i] = 0;
	}

	float RiseTime = DataTable[REG_PULSE_AMPLITUDE] / DataTable[REG_SLEW_RATE];
	float PulseTime = RiseTime + DataTable[REG_PULSE_WIDTH] + RiseTime;

	REGLTR_PulseSamples.TotalSamples = (Int16U)(PulseTime * SAMPLE_RATE);

	REGLTR_PulseSamples.RiseSamples = (Int16U)(REGLTR_PulseSamples.TotalSamples * RiseTime / PulseTime);
	REGLTR_PulseSamples.PlateuSamples = (Int16U)(REGLTR_PulseSamples.TotalSamples * DataTable[REG_PULSE_WIDTH] / PulseTime);
	REGLTR_PulseSamples.FallSamples = REGLTR_PulseSamples.TotalSamples - REGLTR_PulseSamples.RiseSamples - REGLTR_PulseSamples.PlateuSamples;
}
//-----------------------------------------

float REGLTR_GetSetpoint(Int16U i)
{
	if (i < REGLTR_PulseSamples.RiseSamples)
		return (float)i / REGLTR_PulseSamples.RiseSamples * DataTable[REG_PULSE_AMPLITUDE];
	else if (i < REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.PlateuSamples)
		return DataTable[REG_PULSE_AMPLITUDE];
	else if (i < REGLTR_PulseSamples.TotalSamples)
	{
		Int16U fallIdx = i - (REGLTR_PulseSamples.RiseSamples + REGLTR_PulseSamples.PlateuSamples);
		return DataTable[REG_PULSE_AMPLITUDE] * (1.0f - (float)fallIdx / REGLTR_PulseSamples.FallSamples);
	}
	
	return 0;
}
//-----------------------------------------

SamplingResult REGLTR_GetSample()
{
	SamplingResult t = {0};
	Int32U sumUg = 0, sumUPot = 0, sumIg = 0;

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
