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

// Variables
//
static Int64U Timeout = 0;
Int16U LOGIC_ChannelNumber = 0;
// Forward functions
//
void LOGIC_StopProcess();
void LOGIC_SwitchChannels(float Ig);
void LOGIC_SingleSw(float Ig);
// Functions
//

void LOGIC_HandleMeasurement()
{
	static float UgResult, UpotResult, IgResult;

	if(CONTROL_State == DS_InProcess)
	{
		switch (CONTROL_SubState)
		{
			case SS_Init:
				GPIO_SetState(GPIO_VCC_48, true);
				UgResult = UpotResult = IgResult = 0.0f;
				switch(CONTROL_MeasureType)
				{
					case MT_Rth:
						LL_SetCurrentChannel(I_CHANNEL_1);
						LOGIC_ChannelNumber = I_CHANNEL_1;
						break;

					case MT_Iges:
						LL_SetCurrentChannel(I_CHANNEL_5);
						LOGIC_ChannelNumber = I_CHANNEL_5;
						break;

					case MT_Ugeth:
						if(DataTable[REG_WORK_CURRENT_UGETH] < DataTable[REG_RANGE_I_0])
						{
							LL_SetCurrentChannel(I_CHANNEL_1);
							LOGIC_ChannelNumber = I_CHANNEL_1;
						}
						else
						{
							LL_SetCurrentChannel(I_CHANNEL_0);
							LOGIC_ChannelNumber = I_CHANNEL_0;
						}
				}
				Timeout = CONTROL_TimeCounter + INIT_48V_TIMER;
				CONTROL_SetDeviceSubState(SS_Wait48VPause);
				break;

			case SS_Wait48VPause:
				if(CONTROL_TimeCounter > Timeout)
					CONTROL_SetDeviceSubState(SS_ConfigPulse);
				break;

			case SS_ConfigPulse:
				REGLTR_Init();
				REGLTR_StartProcess();
				Timeout = CONTROL_TimeCounter + REGLTR_TIMER;
				CONTROL_SetDeviceSubState(CONTROL_MeasureType == MT_Ugeth ? SS_RegulatorProcessUgeth : SS_RegulatorProcess);
				break;

			case SS_RegulatorProcess:
				if(CONTROL_TimeCounter > Timeout)
				{
					UgResult = Sample.Ug;
					UpotResult = Sample.UPot;
					IgResult = Sample.Ig;
					if(IsMeasureOk)
						LOGIC_SwitchChannels(IgResult);
				}
				break;

			case SS_RegulatorProcessUgeth:
				if(CONTROL_TimeCounter > (Timeout + DataTable[REG_FLATTOP_DURATION]))
				{
					UgResult = Sample.Ug;
					UpotResult = Sample.UPot;
					IgResult = Sample.Ig;
					if(IsMeasureOk)
						CONTROL_SetDeviceSubState(SS_FinishProcess);
				}
				break;
			case SS_FollowingErr:
				LOGIC_StopProcess();
				CONTROL_SwitchToProblem(PROBLEM_FOLLOWING_ERROR);
				break;

			case SS_VoltageErr:
				LOGIC_StopProcess();
				CONTROL_SwitchToProblem(PROBLEM_VOLTAGE_OUT_OF_RANGE);
				break;

			case SS_CurrentErr:
				LOGIC_StopProcess();
				CONTROL_SwitchToProblem(PROBLEM_CURRENT_OUT_OF_RANGE);
				break;

			case SS_FinishProcess:
				LOGIC_StopProcess();
				Timeout = CONTROL_TimeCounter + INIT_48V_TIMER;
				CONTROL_SetDeviceSubState(SS_GetResults);
				break;

			case SS_GetResults:
				if(CONTROL_TimeCounter > Timeout)
				{
					DataTable[REG_THERM_RESIS] = MEASURE_Resis(UgResult, IgResult);
					DataTable[REG_THERM_CURRENT] = IgResult;
					DataTable[REG_UGE_TH] = UgResult;

					CONTROL_SetDeviceState(DS_Ready);
					CONTROL_SetDeviceSubState(SS_None);
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
	LL_SetCurrentChannel(I_CHANNEL_0);
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
	if(Ig < DataTable[REG_RANGE_I_0 + LOGIC_ChannelNumber])
	{
		LL_SetCurrentChannel(LOGIC_ChannelNumber + 1);
		Timeout = CONTROL_TimeCounter + DataTable[REG_PULSE_WIDTH];
		LOGIC_ChannelNumber++;
	}
	else
		CONTROL_SetDeviceSubState(SS_FinishProcess);
}
//------------------------------------------
