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
#include "RingBuffer.h"
#include "Math.h"

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, KiI, KpI, Qi = 0, FollowingErrThreshold, VoltagErrThreshold, CurrentErrThreshold;
static Int16U FollowingErrLimit, VoltagErrLimit, VoltageErrCount, CurrentErrLimit,CurrentErrCount ,ScalingCoef, ScalingCounter, FollowingErrorCounter = 0;
static float PulseAmplitude, DesiredCurrent;
float RawSetPoint = 0;
float VoltStep = 0, Qp = 0;
float RegulatorError = 0;
RegulatorState RegState = RS_None;
volatile bool IsMeasureOk = false;

volatile SamplingResult Sample = {0};

// Forward functions
SamplingResult REGLTR_GetSample();
void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error, float DACRaw);
Int16U REGLTR_CorrectionLogDACPoint();
void RGLTR_ErrorCheck();
Int16U REGLTR_GetScalingCoef();

// Functions
void REGLTR_Process()
{
	if (CONTROL_SubState != SS_RegulatorProcess && CONTROL_SubState != SS_RegulatorProcessUgeth)
		return;

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
				DACSetpoint = REGLTR_CorrectionLogDACPoint();
				LL_WriteDAC(DACSetpoint);
			}
			break;

		case RS_FlatTopUgeth:
		case RS_FlatTop:
		default:
			DACSetpoint = REGLTR_CorrectionLogDACPoint();
			LL_WriteDAC(DACSetpoint);
			break;
	}
}
//-----------------------------------------

void REGLTR_Init()
{
	IsMeasureOk = false;
	Qi = FollowingErrorCounter = VoltageErrCount = CurrentErrCount = 0;
	RINGBUF_ResetIgesAvg();
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
	DataTable[REG_DEBUG_SCALING_COEF] = ScalingCoef = REGLTR_GetScalingCoef();
	if(DataTable[REG_SCALING_MUTE])
		ScalingCoef = 1;
}
//-----------------------------------------

Int16U REGLTR_CorrectionLogDACPoint()
{
	RGLTR_ErrorCheck();

	Qp = RegulatorError * (RegState == RS_FlatTopUgeth ? KpI : Kp);
	Qi += RegulatorError * (RegState == RS_FlatTopUgeth ? KiI : Ki);

	float SetPoint = RawSetPoint + Qp + Qi;
	Int16U DACPoint = MEASURE_ConvertUset(SetPoint);

	REGLTR_StoreRegulatorDebug(Sample.Ug, Sample.UPot, Sample.Ig, RawSetPoint, Qp + Qi, RegulatorError, (float)DACPoint);

	return DACPoint;
}
//-----------------------------------------

void RGLTR_ErrorCheck()
{
	float CurrentErr, VoltageErr;
	switch(RegState)
	{
		case RS_FlatTopUgeth:
			{
				RegulatorError = DesiredCurrent - Sample.Ig;
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

		case RS_FlatTop:
			{
				RegulatorError = RawSetPoint - Sample.UPot;
				// Расчет ошибки по напряжению
				VoltageErr = ABS(Sample.UPot - PulseAmplitude);

				if(VoltageErr < VoltagErrThreshold)
				{
					if(CONTROL_MeasureType == MT_Iges)
						RINGBUF_AddNewSampleIges(Sample.Ig);
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

		default:
			RegulatorError = RawSetPoint - Sample.UPot;
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

void REGLTR_StoreRegulatorDebug(float Ug, float UPot, float Ig, float Setpoint, float Correction, float Error, float DACRaw)
{
	if(ScalingCounter > 1)
		ScalingCounter--;
	else
	{
		ScalingCounter = ScalingCoef;
		if(CONTROL_Values_Counter < VALUES_DEBUG_RGLTR_SIZE)
		{
			CONTROL_RegulatorUg[CONTROL_Values_Counter] = Ug;
			CONTROL_RegulatorUpot[CONTROL_Values_Counter] = UPot;
			CONTROL_RegulatorIg[CONTROL_Values_Counter] = Ig;
			CONTROL_RegulatorSetpoint[CONTROL_Values_Counter] = Setpoint;
			CONTROL_RegulatorCorrection[CONTROL_Values_Counter] = Correction;
			CONTROL_RegulatorError[CONTROL_Values_Counter] = Error;
			CONTROL_DACRaw[CONTROL_Values_Counter] = DACRaw;
			++CONTROL_Values_Counter;
		}
	}
}
//-----------------------------------------

Int16U REGLTR_GetScalingCoef()
{
	Int16U Coef = 0;
	float SingleRelayTimer, RisingPart, SumTicks = 0;
	RisingPart = PulseAmplitude / DataTable[REG_SLEW_RATE];
	switch(CONTROL_MeasureType)
	{
		case MT_Rth:
			SingleRelayTimer = (DataTable[REG_RELAY_SW_TIMER_RTH] > DataTable[REG_REGLTR_TIMER]) ?
								DataTable[REG_RELAY_SW_TIMER_RTH] : DataTable[REG_REGLTR_TIMER];
			SumTicks = (RisingPart + 3 * SingleRelayTimer) / TIMER15_uS;
			break;

		case MT_Iges:
			SingleRelayTimer =(DataTable[REG_RELAY_SW_TIMER_IGES] > DataTable[REG_REGLTR_TIMER]) ?
							   DataTable[REG_RELAY_SW_TIMER_IGES] : DataTable[REG_REGLTR_TIMER];
			SumTicks = (RisingPart + 3 * SingleRelayTimer) / TIMER15_uS;
			break;

		case MT_Ugeth:
			SingleRelayTimer = (DataTable[REG_RELAY_SW_TIMER_UGETH] > DataTable[REG_REGLTR_TIMER]) ?
								DataTable[REG_RELAY_SW_TIMER_UGETH] : DataTable[REG_REGLTR_TIMER];
			SumTicks = (RisingPart + 2 * SingleRelayTimer) / TIMER15_uS;
			break;
	}
	if (fmod(SumTicks ,(float)VALUES_DEBUG_RGLTR_SIZE) > 1)
		Coef = (Int16U)ceil(SumTicks / (float)VALUES_DEBUG_RGLTR_SIZE);
	else
		Coef = 1;
	ScalingCounter = Coef;
	return Coef;
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
	TIM_Stop(TIM15);

	DMA_ChannelEnable(DMA1_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel1, false);
	DMA_ChannelEnable(DMA2_Channel5, false);
}
//------------------------------------
