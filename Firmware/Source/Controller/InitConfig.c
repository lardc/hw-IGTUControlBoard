#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"
#include "ZwSPI.h"
#include "Diagnostic.h"

// Definition
//
#define EXTI8_INTERRUPT_PRIORITY	27
#define EXTI15_INTERRUPT_PRIORITY	30
#define TIM3_INTERRUPT_PRIORITY		36
#define USART1_INTERRUPT_PRIORITY	44
#define CAN_INTERRUPT_PRIORITY		47

// Functions
//
Boolean INITCFG_SysClk()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_EI()
{
	EXTI_Config(EXTI_PA, EXTI_8, BOTH_TRIG, 0);
	EXTI_Config(EXTI_PB, EXTI_15, BOTH_TRIG, 0);
	//
	EXTI_EnableInterrupt(EXTI9_5_IRQn, 0, true);
	EXTI_EnableInterrupt(EXTI15_10_IRQn, 0, true);

	NVIC_SetPriority(EXTI9_5_IRQn, EXTI8_INTERRUPT_PRIORITY);
	NVIC_SetPriority(EXTI15_10_IRQn, EXTI15_INTERRUPT_PRIORITY);
}
//------------------------------------------------

void INITCFG_IO()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	
	// Аналаговые входы
	GPIO_Config(GPIOA, Pin_3, Analog, NoPull, HighSpeed, NoPull);
	GPIO_Config(GPIOA, Pin_14, Analog, NoPull, HighSpeed, NoPull);
	
	// Выходы
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_LED_EXT);
	GPIO_InitPushPullOutput(GPIO_VCC_24);
	GPIO_InitPushPullOutput(GPIO_VCC_48);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_CAN_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_UART_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_SPI_CLK, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_SPI_MOSI, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_SPI_SS, AltFn_5);
}
//------------------------------------------------

void INITCFG_UART()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
	NVIC_SetPriority(RTC_Alarm_IRQn, USART1_INTERRUPT_PRIORITY);
}
//------------------------------------------------

void INITCFG_ADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);
	
	ADC_Calibration(ADC1);
	ADC_SoftTrigConfig(ADC1);
	ADC_Enable(ADC1);
}
//------------------------------------------------

void INITCFG_DAC1()
{
	DACx_Clk_Enable(DAC_1_ClkEN);
	DACx_Reset();
	DAC_Trigger_Config(TRIG1_TIMER6, TRIG1_ENABLE);
	DAC_Buff(BUFF1, false);
	DACx_DMA_Config(DAC_DMA1ENABLE, DAC_DMA1UdIntDISABLE);
	DACx_Enable(DAC1ENABLE);
}

void INITCFG_Timer7()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 0, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void INITCFG_Timer3()
{
	TIM_Clock_En(TIM_3);
	TIM_Config(TIM3, SYSCLK, TIMER3_uS);
	TIM_Interupt(TIM3, 0, true);
	NVIC_SetPriority(EXTI15_10_IRQn, TIM3_INTERRUPT_PRIORITY);
	TIM_Stop(TIM3);
}
//------------------------------------------------

void INITCFG_Timer6()
{
	TIM_Clock_En(TIM_6);
	TIM_Config(TIM6, SYSCLK, TIMER6_uS);
	TIM_MasterMode(TIM6, MMS_UPDATE);
	TIM_DMA(TIM6, DMAEN);
}

void INITCFG_WatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigCAN(Int16U NodeID)
{
	Int32U Mask = ((Int32U)NodeID) << CAN_SLAVE_NID_MPY;

	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, FALSE);
	NCAN_FIFOInterrupt(TRUE);
	NCAN_FilterInit(0, Mask, Mask);
	NCAN_InterruptSetPriority(0);
}
//------------------------------------------------

void INITCFG_SPI()
{
	SPI_Init(SPI1, SPI_BAUDRATE_BITS, SPI_LSB_FIRST);
}
//------------------------------------------------

void INITCFG_DMA(uint16_t Size)
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_TIM6DAC1Ch1_DMA_RMP;

	DMA_Clk_Enable(DMA_ClkEN);
	DMA_Reset(DMA1_Channel3);
	DMA_Interrupt(DMA1_Channel3, DMA_TRANSFER_COMPLETE, 2, true);

	DMA1ChannelX_DataConfig(
		DMA1_Channel3,
		(uint32_t)(&DIAG_PulseDataBuffer),
		(uint32_t)(&DAC->DHR12R1),
		Size
	);

	DMA1ChannelX_Config(DMA1_Channel3, DMA_MEM2MEM_DIS, DMA_LvlPriority_LOW, DMA_MSIZE_16BIT, DMA_PSIZE_16BIT,
	DMA_MINC_EN, false, DMA_CIRCMODE_EN, DMA_READ_FROM_MEM, DMA_CHANNEL_EN);
}
