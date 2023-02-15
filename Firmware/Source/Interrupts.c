// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Board.h"
#include "SysConfig.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

// Functions
//
void USART1_IRQHandler()
{
	if(ZwSCI_RecieveCheck(USART1))
	{
		ZwSCI_RegisterToFIFO(USART1);
		ZwSCI_RecieveFlagClear(USART1);
	}
}
//-----------------------------------------

void USB_LP_CAN_RX0_IRQHandler()
{
	if(NCAN_RecieveCheck())
	{
		NCAN_RecieveData();
		NCAN_RecieveFlagReset();
	}
}
//-----------------------------------------

void TIM1_BRK_TIM15_IRQHandler()
{
	if(TIM_StatusCheck(TIM15))
	{
		CONTROL_HighPriorityProcess();

		TIM_StatusClear(TIM15);
	}
}
//-----------------------------------------

void TIM7_IRQHandler()
{
	static uint16_t LED_BlinkTimeCounter = 0;

	if(TIM_StatusCheck(TIM7))
	{
		CONTROL_TimeCounter++;

		if(++LED_BlinkTimeCounter > TIME_LED_BLINK)
		{
			LL_ToggleBoardLED();
			LED_BlinkTimeCounter = 0;
		}

		TIM_StatusClear(TIM7);
	}
}
//-----------------------------------------

void EXTI2_TSC_IRQHandler()
{
	EXTI_FlagReset(EXTI_2);
}
//-----------------------------------------

void EXTI15_10_IRQHandler()
{
	if(CONTROL_State == DS_InProcess && !DataTable[REG_MUTE_SAFETY_MONITOR])
	{
		CONTROL_ForceStopProcess();

		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_SAFETY_VIOLATION;

		CONTROL_SetDeviceState(DS_Ready, SS_None);
	}

	EXTI_FlagReset(EXTI_15);
}
//-----------------------------------------
