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
#include "Utils.h"

// Variables
//
float RelayLimits[7]; // РњРёРЅ. Р·РЅР°С‡РµРЅРёСЏ РґРёР°РїР°Р·РѕРЅРѕРІ СЂР°Р±РѕС‚С‹ СЂРµР»Рµ РѕС‚ 0 РґРѕ 6, РІ Рђ

// Forward functions
//

// Functions
//

void LOGIC_HandleMeasurement()
{
	static Int64U Timeout = 0;
	static float UgResult, UpotResult, IgResult;
	static Int16U VoltageErrCount;

	if(CONTROL_State == DS_InProcess)
	{
		switch (CONTROL_SubState)
		{
			case SS_Init:
				GPIO_SetState(GPIO_VCC_48, true);
				UgResult = UpotResult = IgResult = 0.0f;
				VoltageErrCount = 0;
				LL_SetCurrentChannel((CONTROL_MeasureType == MT_Rth) ? I_CHANNEL_6 : I_CHANNEL_2);
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

				for (int i = 0; i<7;i++)
					RelayLimits[i] = DataTable[REG_RANGE_I_0 + i];

				CONTROL_SetDeviceSubState(SS_RegulatorProcess);
				break;

			case SS_RegulatorProcess:
				if(CONTROL_TimeCounter > Timeout)
				{
					SamplingResult Result = REGLTR_GetSample();
					UgResult = Result.Ug;
					UpotResult = Result.UPot;
					IgResult = Result.Ig;
					switch(CONTROL_MeasureType)
					{
						case MT_Iges:
						{
							float VoltageErr = ABS(UgResult - DataTable[REG_WORK_VOLTAGE_IGES]);
							if(VoltageErr < DataTable[REG_VOLTAGE_ERR_LIMIT])
							{
								for(int i = 5; i <= 6; i++)
								{
									if(IgResult < RelayLimits[i])
									{
										LL_SetCurrentChannel(i + 1);
										Timeout = CONTROL_TimeCounter + SW_CURRENT_CH_TIMER;
									}
									else
									{
										CONTROL_SetDeviceSubState(SS_FinishProcess);
										break;
									}
								}
							}
							else
							{
								VoltageErrCount++;
								if(VoltageErrCount > DataTable[REG_VOLTAGE_ERR_COUNT_LIMIT])
									CONTROL_SwitchToProblem(PROBLEM_VOLTAGE_OUT_OF_RANGE);
							}
						}
						break;

						case MT_Rth:
						{
							float VoltageErr = ABS(UgResult - DataTable[REG_WORK_VOLTAGE_RTH]);
							if(VoltageErr < DataTable[REG_VOLTAGE_ERR_LIMIT])
							{
								for(int i = 2; i < 5; i++)
								{
									if(IgResult < RelayLimits[i])
									{
										LL_SetCurrentChannel(i + 1);
										Timeout = CONTROL_TimeCounter + SW_CURRENT_CH_TIMER;
									}
									else
										CONTROL_SetDeviceSubState(SS_FinishProcess);
								}
							}
							else
							{
								VoltageErrCount++;
								if(VoltageErrCount > DataTable[REG_VOLTAGE_ERR_COUNT_LIMIT])
									CONTROL_SwitchToProblem(PROBLEM_VOLTAGE_OUT_OF_RANGE);
							}
						}
						break;
					}
				}
				break;

			case SS_FinishProcess:
				REGLTR_StopProcess();
				GPIO_SetState(GPIO_VCC_48, false);
				LL_SetCurrentChannel(I_CHANNEL_0);
				Timeout = CONTROL_TimeCounter + INIT_48V_TIMER;
				CONTROL_SetDeviceSubState(SS_GetResults);
				break;

			case SS_GetResults:
				if(CONTROL_TimeCounter > Timeout)
				{
					DataTable[REG_THERM_RESIS] = MEASURE_Resis(UgResult, IgResult);
					DataTable[REG_THERM_CURRENT] = IgResult;

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
