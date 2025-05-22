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
//

// Variables
//
bool Impulse = false;

// Functions prototypes
void INT_SyncTimeoutControl(bool State);
//

// Functions
//
void EXTI9_5_IRQHandler()
{
	EXTI_FlagReset(EXTI_8);
}
//-----------------------------------------

void EXTI15_10_IRQHandler()
{
	EXTI_FlagReset(EXTI_15);
}
//-----------------------------------------

void TIM3_IRQHandler()
{
	if(TIM_StatusCheck(TIM3))
	{
		INT_SyncTimeoutControl(false);
		TIM_StatusClear(TIM3);
	}
}
//-----------------------------------------

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

void INT_SyncTimeoutControl(bool State)
{
	State ? TIM_Start(TIM3) : TIM_Stop(TIM3);
	TIM_Reset(TIM3);
}
//-----------------------------------------
