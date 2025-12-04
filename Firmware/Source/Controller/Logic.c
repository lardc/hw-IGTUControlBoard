// Header
//
#include "Logic.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Controller.h"
#include "Board.h"

// Types
//

// Variables
//

// Forward functions
//

// Functions
//

void LOGIC_HandlePowerOn()
{
	static Int64U Timeout = 0;

	if(CONTROL_State == DS_InProcess)
	{
		switch (CONTROL_SubState)
		{
			case SS_PowerOn:
				Timeout = CONTROL_TimeCounter + POWER_ON_TIMER;
				GPIO_SetState(GPIO_VCC_48, true);
				CONTROL_SetDeviceState(DS_InProcess);
				CONTROL_SetDeviceSubState(SS_WaitCharge);
				break;

			case SS_WaitCharge:
				if(CONTROL_TimeCounter > Timeout)
				{
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
