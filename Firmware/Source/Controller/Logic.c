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

// Types
//

// Variables
//

// Forward functions
//

// Functions
//

void LOGIC_HandleMeasurement()
{
	if(CONTROL_State == DS_InProcess)
	{
		switch (CONTROL_SubState)
		{
			case SS_Init:

				LL_SetCurrentChannel(I_CHANNEL_7);
				CONTROL_SetDeviceSubState(SS_None);
				break;


			default:
				break;
		}
	}
}
//------------------------------------------
