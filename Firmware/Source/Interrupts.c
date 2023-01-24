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

void TIM15_IRQHandler()
{
	if(TIM_StatusCheck(TIM15))
	{
		CONTROL_V_HighPriorityProcess();

		TIM_StatusClear(TIM15);
	}
}
//-----------------------------------------

void TIM4_IRQHandler()
{
	if(TIM_StatusCheck(TIM4))
	{
		if(++CONTROL_C_TimeCounter > (Int16U)((float)DataTable[REG_QG_T_CURRENT] / (float)TIMER4_uS))
			CONTROL_C_HighPriorityProcess(false);
		else
			CONTROL_C_HighPriorityProcess(true);
		TIM_StatusClear(TIM4);
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
