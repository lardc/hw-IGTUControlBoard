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

void EXTI15_10_IRQHandler(void)
{
	/*if (LL_ICompState())
		CONTROL_C_HighPriorityProcess(true, true);
	else
		CONTROL_C_HighPriorityProcess(false, true);
	EXTI_FlagReset(EXTI_15);*/
}
//-----------------------------------------------

void TIM15_IRQHandler()
{
	if(TIM_StatusCheck(TIM15))
	{
		CONTROL_VGS_HighPriorityProcess();

		TIM_StatusClear(TIM15);
	}
}
//-----------------------------------------

void TIM6_IRQHandler()
{
	if(TIM_StatusCheck(TIM6))
	{
		if(++CONTROL_I_TimeCounter > (Int16U)((float)DataTable[REG_QG_T_CURRENT] / (float)TIMER6_uS))
			CONTROL_C_HighPriorityProcess(false, false);
		else
			CONTROL_C_HighPriorityProcess(true, false);
		TIM_StatusClear(TIM6);
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

void DMA1_Channel1_IRQHandler()
{
	/*if (DMA_IsTransferComplete(DMA1, DMA_ISR_TCIF1))
	{
		TIM_Stop(TIM6);
		TIM_Reset(TIM6);

		ADC_SamplingStop(ADC1);

		DMA_TransferCompleteReset(DMA1, DMA_ISR_TCIF1);
	}*/
}
//-----------------------------------------
