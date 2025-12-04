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

// Vаriables
//

// Forward functions
//

// Functions
//

void LOGIC_HandleMeasurement()
{
	static Int64U Timeout = 0;
	static float UgResult, UpotResult, IgResult;
	if(CONTROL_State == DS_InProcess)
	{
		switch (CONTROL_SubState)
		{
			case SS_Init:
				GPIO_SetState(GPIO_VCC_48, true);
				UgResult = UpotResult = IgResult = 0.0f;
				//LL_SetCurrentChannel(I_CHANNEL_7);
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
				CONTROL_SetDeviceSubState(SS_RegulatorProcess);
				break;

			case SS_RegulatorProcess:
				if(CONTROL_TimeCounter > Timeout)
				{
					UgResult = REGLTR_GetSample().Ug;
					UpotResult = REGLTR_GetSample().UPot;
					IgResult = REGLTR_GetSample().Ig;
					CONTROL_SetDeviceSubState(SS_FinishProcess);
				}
				break;

			case SS_FinishProcess:
				REGLTR_StopProcess();
				GPIO_SetState(GPIO_VCC_48, false);
				//LL_SetCurrentChannel(I_CHANNEL_NONE);
				DataTable[REG_THERM_RESIS] = MEASURE_Resis(UgResult, IgResult);
				DataTable[REG_THERM_CURRENT] = IgResult;

				CONTROL_SetDeviceState(DS_Ready);
				CONTROL_SetDeviceSubState(SS_None);
				break;

			default:
				break;
		}
	}
}
//------------------------------------------
