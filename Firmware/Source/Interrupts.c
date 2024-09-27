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
#include "Qg.h"
#include "Iges.h"
#include "Delay.h"

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
		CONTROL_HandleExternalLamp(true);

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

		CONTROL_HandleExternalLamp(IsImpulse);

		TIM_StatusClear(TIM7);
	}
}
//-----------------------------------------

void TIM3_IRQHandler()
{
	if(CONTROL_SubState == SS_QgProcess)
	{
		QG_Pulse(false);

		if(CONTROL_State == DS_SelfTest)
		{
			CONTROL_ResetHardwareToDefaultState();
			QG_SaveResult();
			CONTROL_SetDeviceState(DS_SelfTest, SS_I_Check);
		}
		else
		{
			LL_SyncTOCUHP(false);
			DELAY_US(100);

			CONTROL_SetDeviceState(DS_InProcess, SS_QgSaveResult);
		}
	}

	TIM_StatusClear(TIM3);
}
//-----------------------------------------

void EXTI2_TSC_IRQHandler()
{
	PAU_SyncFlag = true;
	EXTI_FlagReset(EXTI_2);
}
//-----------------------------------------

void EXTI15_10_IRQHandler()
{
	CONTROL_IsSafetyOk();
	EXTI_FlagReset(EXTI_15);
}
//-----------------------------------------
