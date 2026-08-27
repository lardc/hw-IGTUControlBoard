// Header
//
#include "Logic.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"
#include "Board.h"
#include "LowLevel.h"
#include "Regulator.h"
#include "Measurement.h"
#include "RingBuffer.h"
#include "Utils.h"
#include "Constraints.h"

// Variables
//
static Int64U Timeout = 0;
Int16U LOGIC_ChannelNumber = 0;
Int16U RelaySwitchTimer = 0;
static Int16U ForcedCh = 0;
static Boolean SecondaryST = false;
// Forward functions
//
void LOGIC_StopProcess();
void LOGIC_SwitchChannels(float Ig);
void LOGIC_SingleSw(float Ig);
void LOGIC_TestLoadRelaySwitch();
Boolean LOGIC_IsSelfTest();
void LOGIC_ErrorHandler(DeviceSubState SubState);
// Functions
//

void LOGIC_HandleMeasurement()
{
	static float UgResult, UpotResult, IgResult;

	if(CONTROL_State == DS_InProcess)
	{
		if(!CONTROL_IsSafetyOk())
		{
			LOGIC_StopProcess();
			SecondaryST = false;
		}

		switch(CONTROL_SubState)
		{
			case SS_Init:
				UgResult = UpotResult = IgResult = 0.0f;
				ForcedCh = DataTable[REG_DIAG_FORCE_CHANNEL];

				switch(CONTROL_MeasureType)
				{
					case MT_Rth:
						if(ForcedCh && ForcedCh != I_CHANNEL_0 && ForcedCh != I_CHANNEL_1 && ForcedCh != I_CHANNEL_2
								&& ForcedCh != I_CHANNEL_3)
						{
							CONTROL_SwitchToProblem(PROBLEM_WRONG_SELECTED_RELAY);
							return;
						}

						GPIO_SetState(GPIO_VCC_24, true);

						LOGIC_ChannelNumber = ForcedCh ? ForcedCh : I_CHANNEL_0;
						LL_SetCurrentChannel(LOGIC_ChannelNumber);
						RelaySwitchTimer = DataTable[REG_RELAY_SW_TIMER_RTH];
						break;

					case MT_Iges:
						if(ForcedCh == I_CHANNEL_0)
						{
							CONTROL_SwitchToProblem(PROBLEM_WRONG_SELECTED_RELAY);
							return;
						}
						LL_SetNegativePolarity(DataTable[REG_WORK_VOLTAGE_IGES] < 0);
						GPIO_SetState(GPIO_VCC_48, true);

						LOGIC_ChannelNumber = ForcedCh ? ForcedCh : I_CHANNEL_5;
						LL_SetCurrentChannel(LOGIC_ChannelNumber);

						// Для форсированного канала 7 выбирается отдельная задержка
						RelaySwitchTimer = DataTable[
								(LOGIC_ChannelNumber == I_CHANNEL_7) ?
										REG_RELAY_SW_TIMER_IGES_RANGE7 : REG_RELAY_SW_TIMER_IGES];
						break;

					case MT_Ugeth:
						if(ForcedCh && ForcedCh != I_CHANNEL_1 && ForcedCh != I_CHANNEL_0)
						{
							CONTROL_SwitchToProblem(PROBLEM_WRONG_SELECTED_RELAY);
							return;
						}

						GPIO_SetState(GPIO_VCC_24, true);
						RelaySwitchTimer = DataTable[REG_RELAY_SW_TIMER_UGETH];

						if(ForcedCh)
						{
							LL_SetCurrentChannel(ForcedCh);
							LOGIC_ChannelNumber = ForcedCh;
						}
						else if((DataTable[REG_WORK_CURRENT_UGETH] * 0.001f) < DataTable[REG_RANGE_I_0])
						{
							LL_SetCurrentChannel(I_CHANNEL_1);
							LOGIC_ChannelNumber = I_CHANNEL_1;
						}
						else
						{
							LL_SetCurrentChannel(I_CHANNEL_0);
							LOGIC_ChannelNumber = I_CHANNEL_0;
						}
						break;

					case MT_ST_Upot:
						GPIO_SetState(GPIO_VCC_24, true);
						LL_SetSelfTestUpot(true);
						RelaySwitchTimer = DataTable[REG_REGLTR_TIMER] + DataTable[REG_ST_UPOT_FLATTOP_DURATION];
						LL_SetCurrentChannel(I_CHANNEL_0);
						LOGIC_ChannelNumber = I_CHANNEL_0;
						if(DataTable[REG_USE_SELFTEST] == BOTH_ST)
							SecondaryST = true;
						break;

					case MT_ST_TestLoad:
						GPIO_SetState(GPIO_VCC_48, true);
						RelaySwitchTimer = DataTable[REG_REGLTR_TIMER] + DataTable[REG_ST_TL_FLATTOP_DURATION];
						LL_SetSelfTestLoad(true);
						LOGIC_TestLoadRelaySwitch();
						break;
				}
				Timeout = CONTROL_TimeCounter + TIME_INIT_PS_TIMER;
				CONTROL_SetDeviceSubState(SS_WaitPowerSupply);
				break;

			case SS_WaitPowerSupply:
				if(CONTROL_TimeCounter > Timeout)
					CONTROL_SetDeviceSubState(SS_ConfigPulse);
				break;

			case SS_ConfigPulse:
				REGLTR_Init();
				REGLTR_StartProcess();
				LL_Sync(true);
				Timeout = CONTROL_TimeCounter + MAX(RelaySwitchTimer, DataTable[REG_REGLTR_TIMER]);

				if(CONTROL_MeasureType == MT_Ugeth)
					CONTROL_SetDeviceSubState(SS_RegulatorProcessUgeth);
				else if(CONTROL_MeasureType == MT_ST_Upot || CONTROL_MeasureType == MT_ST_TestLoad)
					CONTROL_SetDeviceSubState(SS_RegulatorProcessSelfTest);
				else
					CONTROL_SetDeviceSubState(SS_RegulatorProcess);
				break;

			case SS_RegulatorProcess:
				if(CONTROL_TimeCounter > Timeout)
				{
					UgResult = Sample.Ug;
					UpotResult = Sample.UPot;
					IgResult = Sample.Ig;
					if(IsMeasureOk)
					{
						if(ForcedCh)
							CONTROL_SetDeviceSubState(SS_FinishProcess);
						else
						{
							if(CONTROL_MeasureType == MT_Rth)
								REGLTR_SetPause();
							else if(CONTROL_MeasureType == MT_Iges && LOGIC_ChannelNumber == I_CHANNEL_6)
								// Перед включением канала 7 выставляем задержку из выделенного регистра
								RelaySwitchTimer = DataTable[REG_RELAY_SW_TIMER_IGES_RANGE7];

							LOGIC_SwitchChannels(IgResult);
						}
					}
				}
				break;

			case SS_RegulatorProcessSelfTest:
				if(CONTROL_TimeCounter > Timeout)
					if(IsMeasureOk)
					{
						UgResult = Sample.Ug;
						UpotResult = Sample.UPot;
						IgResult = Sample.Ig;
						DataTable[REG_OP_RESULT] = OPRESULT_OK;
						CONTROL_SetDeviceSubState(SS_FinishProcess);
					}
				break;

			case SS_RegulatorProcessUgeth:
				if(CONTROL_TimeCounter > (Timeout + DataTable[REG_CURRENT_FLATTOP_DURATION]))
				{
					UgResult = Sample.Ug;
					UpotResult = Sample.UPot;
					IgResult = Sample.Ig;
					if(IsMeasureOk)
						CONTROL_SetDeviceSubState(SS_FinishProcess);
				}
				break;
			case SS_FollowingErr:
			case SS_FollowingErrUpot:
			case SS_VoltageErr:
			case SS_CurrentErr:
				LOGIC_ErrorHandler(CONTROL_SubState);
				break;

			case SS_VoltageNoCurrentErr:
				LOGIC_StopProcess();
				CONTROL_SwitchToProblem(PROBLEM_VOLTAGE_LIMIT_NO_CURRENT);
				break;

			case SS_FinishProcess:
				LOGIC_StopProcess();
				Timeout = CONTROL_TimeCounter + TIME_INIT_PS_TIMER;
				CONTROL_SetDeviceSubState(SS_GetResults);
				break;

			case SS_GetResults:
				if(CONTROL_TimeCounter > Timeout)
				{
					bool MainMeasurement = CONTROL_MeasureType == MT_Rth || CONTROL_MeasureType == MT_Iges
							|| CONTROL_MeasureType == MT_Ugeth;
					bool ResultOk = RINGBUF_IsFull() || !MainMeasurement;

					if(ResultOk)
					{
						float AvgI = RINGBUF_GetAvgI();
						float AvgU = RINGBUF_GetAvgU();
						float R;

						switch(CONTROL_MeasureType)
						{
							case MT_Rth:
								R = (AvgI == 0.0f) ? 0.0f : (AvgU / AvgI);
								if(R > DataTable[REG_MAX_RTH_RESISTANCE])
								{
									ResultOk = false;
									DataTable[REG_PROBLEM] = PROBLEM_RTH_TOO_HIGH;
								}
								else if(R < DataTable[REG_MIN_RTH_RESISTANCE])
								{
									ResultOk = false;
									DataTable[REG_PROBLEM] = PROBLEM_RTH_TOO_LOW;
								}
								else
									DataTable[REG_THERM_RESIS] = R;

								if(ResultOk)
								{
									DataTable[REG_DIAG_CURRENT] = AvgI;
									DataTable[REG_DIAG_VOLTAGE] = UgResult;
									DataTable[REG_DIAG_POT_VOLTAGE] = AvgU;
								}
								break;

							case MT_Iges:
								if(AvgI <= DataTable[REG_IGES_MAX_CURRENT])
								{
									DataTable[REG_DIAG_CURRENT] = DataTable[REG_IGES_RESULT] = AvgI;
									DataTable[REG_DIAG_VOLTAGE] = AvgU;
									DataTable[REG_DIAG_POT_VOLTAGE] = UpotResult;
								}
								else
								{
									ResultOk = false;
									DataTable[REG_PROBLEM] = PROBLEM_IGES_TOO_HIGH;
								}
								break;

							case MT_Ugeth:
								DataTable[REG_DIAG_CURRENT] = AvgI;
								DataTable[REG_DIAG_VOLTAGE] = UgResult;
								DataTable[REG_DIAG_POT_VOLTAGE] = DataTable[REG_UGE_TH] = AvgU;
								break;

							default:
								break;
						}
					}
					else
						DataTable[REG_PROBLEM] = PROBLEM_RING_BUFFER_NOT_FILLED;

					// Последние мгновенные значения в случае проблемы или неосновного измерения
					if(!ResultOk || !MainMeasurement)
					{
						DataTable[REG_DIAG_CURRENT] = IgResult;
						DataTable[REG_DIAG_VOLTAGE] = UgResult;
						DataTable[REG_DIAG_POT_VOLTAGE] = UpotResult;
					}
					DataTable[REG_OP_RESULT] = ResultOk ? OPRESULT_OK : OPRESULT_FAIL;

					if(SecondaryST)
					{
						CONTROL_StartMeasure(MT_ST_TestLoad);
						SecondaryST = false;
					}
					else
					{
						CONTROL_SetDeviceState(DS_Ready);
						CONTROL_SetDeviceSubState(SS_None);
					}
				}
				break;

			default:
				break;
		}
	}
}
//------------------------------------------

void LOGIC_StopProcess()
{
	REGLTR_StopProcess();
	GPIO_SetState(GPIO_VCC_48, false);
	GPIO_SetState(GPIO_VCC_24, false);
	LL_SetNegativePolarity(false);
	LL_SetSelfTestLoad(false);
	LL_SetSelfTestUpot(false);
	LL_Sync(false);
	LL_SetCurrentChannel(I_CHANNEL_DEF);
}
//------------------------------------------

Boolean LOGIC_IsSelfTest()
{
	return CONTROL_MeasureType == MT_ST_Upot || CONTROL_MeasureType == MT_ST_TestLoad;
}
//------------------------------------------

void LOGIC_SwitchChannels(float Ig)
{
	switch(LOGIC_ChannelNumber)
	{
		case I_CHANNEL_0:
			LOGIC_SingleSw(Ig);
			break;

		case I_CHANNEL_1:
			LOGIC_SingleSw(Ig);
			break;

		case I_CHANNEL_2:
			LOGIC_SingleSw(Ig);
			break;

		case I_CHANNEL_3:
			CONTROL_SetDeviceSubState(SS_FinishProcess);
			break;

		case I_CHANNEL_5:
			LOGIC_SingleSw(Ig);
			break;

		case I_CHANNEL_6:
			LOGIC_SingleSw(Ig);
			break;

		case I_CHANNEL_7:
			CONTROL_SetDeviceSubState(SS_FinishProcess);
			break;
	}
}
//------------------------------------------

void LOGIC_SingleSw(float Ig)
{
	if(Ig < DataTable[REG_RANGE_I_0 + LOGIC_ChannelNumber - 1])
	{
		LL_SetCurrentChannel(LOGIC_ChannelNumber + 1);
		Timeout = CONTROL_TimeCounter + MAX(RelaySwitchTimer, DataTable[REG_REGLTR_TIMER]);
		LOGIC_ChannelNumber++;
	}
	else
		CONTROL_SetDeviceSubState(SS_FinishProcess);
}
//------------------------------------------

void LOGIC_TestLoadRelaySwitch()
{
	float CalcCurrent = (DataTable[REG_WORK_VOLTAGE_ST_TESTLOAD] * 0.001f) /  DataTable[REG_ST_TESTLOAD_RESIS];
	if(CalcCurrent > DataTable[REG_RANGE_I_0])
	{
		LL_SetCurrentChannel(I_CHANNEL_0);
		LOGIC_ChannelNumber = I_CHANNEL_0;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_1])
	{
		LL_SetCurrentChannel(I_CHANNEL_1);
		LOGIC_ChannelNumber = I_CHANNEL_1;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_2])
	{
		LL_SetCurrentChannel(I_CHANNEL_2);
		LOGIC_ChannelNumber = I_CHANNEL_2;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_3])
	{
		LL_SetCurrentChannel(I_CHANNEL_3);
		LOGIC_ChannelNumber = I_CHANNEL_3;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_4])
	{
		LL_SetCurrentChannel(I_CHANNEL_4);
		LOGIC_ChannelNumber = I_CHANNEL_4;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_5])
	{
		LL_SetCurrentChannel(I_CHANNEL_5);
		LOGIC_ChannelNumber = I_CHANNEL_5;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_6])
	{
		LL_SetCurrentChannel(I_CHANNEL_6);
		LOGIC_ChannelNumber = I_CHANNEL_6;
		return;
	}
	else if(CalcCurrent > DataTable[REG_RANGE_I_7])
	{
		LL_SetCurrentChannel(I_CHANNEL_7);
		LOGIC_ChannelNumber = I_CHANNEL_7;
		return;
	}
}
//------------------------------------------

void LOGIC_ErrorHandler(DeviceSubState SubState)
{
	Int16U FaultReason, ProblemReason;

	switch(SubState)
	{
		case SS_FollowingErr:
			FaultReason = DF_FOLLOWING_ERROR;
			ProblemReason = PROBLEM_FOLLOWING_ERROR;
			break;

		case SS_FollowingErrUpot:
			FaultReason = DF_FOLLOWING_ERROR_UPOT;
			ProblemReason = PROBLEM_FOLLOWING_ERROR_UPOT;
			break;

		case SS_VoltageErr:
			FaultReason = DF_VOLTAGE_OUT_OF_RANGE;
			ProblemReason = PROBLEM_VOLTAGE_OUT_OF_RANGE;
			break;

		case SS_CurrentErr:
			FaultReason = DF_CURRENT_OUT_OF_RANGE;
			ProblemReason = PROBLEM_CURRENT_OUT_OF_RANGE;
			break;

		default:
			return;
	}

	float Ug = Sample.Ug;
	float Upot = Sample.UPot;
	float Ig = Sample.Ig;

	LOGIC_StopProcess();
	SecondaryST = false;

	if(LOGIC_IsSelfTest())
	{
		DataTable[REG_DIAG_CURRENT] = Ig;
		DataTable[REG_DIAG_VOLTAGE] = Ug;
		DataTable[REG_DIAG_POT_VOLTAGE] = Upot;
		CONTROL_SwitchToFault(FaultReason);
	}
	else
		CONTROL_SwitchToProblem(ProblemReason);
}
//------------------------------------------
