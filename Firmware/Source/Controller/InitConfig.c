#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"
#include "Measurement.h"

// Functions
//
Boolean INITCFG_ConfigSystemClock()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_ConfigDAC()
{
	DACx_Clk_Enable(DAC_1_ClkEN);
	DACx_Reset();
	DAC_TriggerConfigCh1(DAC1, TRIG1_TIMER4, TRIG1_ENABLE);
	DAC_BufferCh1(DAC1, true);
	DAC_EnableCh1(DAC1);
	DAC_BufferCh2(DAC1, true);
	DAC_EnableCh2(DAC1);
}
//------------------------------------------------

void INITCFG_ConfigTimer4()
{
	TIM_Clock_En(TIM_4);
	TIM_Config(TIM4, SYSCLK, TIMER4_uS);
	TIM_MasterMode(TIM4, MMS_UPDATE);
	TIM_Start(TIM4);
}
//------------------------------------------------

void INITCFG_ConfigIO()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	RCC_GPIO_Clk_EN(PORTC);
	
	// Выходы
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_INDICATION);
	GPIO_InitPushPullOutput(GPIO_V_CURR_K1);
	GPIO_InitPushPullOutput(GPIO_V_CURR_K2);
	GPIO_InitPushPullOutput(GPIO_V_LOW_CURRENT);
	GPIO_InitPushPullOutput(GPIO_V_HIGH_CURRENT);
	GPIO_InitPushPullOutput(GPIO_C_C_START);
	GPIO_InitPushPullOutput(GPIO_C_ENABLE);
	GPIO_InitPushPullOutput(GPIO_C_DIAG_CTRL);
	GPIO_InitPushPullOutput(GPIO_C_EXT_DAC_CS);
	GPIO_InitPushPullOutput(GPIO_C_EXT_DAC_LDAC);
	GPIO_InitPushPullOutput(GPIO_SYNC_TOCUHP);
	GPIO_InitPushPullOutput(GPIO_SYNC_OSC);
	GPIO_InitPushPullOutput(GPIO_SYNC_PAU);
	GPIO_InitPushPullOutput(GPIO_OUT_MULTIPLEX);

	// Выходы OpenDrain
	GPIO_InitOpenDrainOutput(GPIO_V_DIAG_CTRL, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_V_SHORT_PAU, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_V_SHORT_OUT, NoPull);

	// Начальная установка состояний выходов
	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_INDICATION, false);
	GPIO_SetState(GPIO_V_CURR_K1, false);
	GPIO_SetState(GPIO_V_CURR_K2, true);
	GPIO_SetState(GPIO_V_LOW_CURRENT, false);
	GPIO_SetState(GPIO_V_HIGH_CURRENT, true);
	GPIO_SetState(GPIO_V_DIAG_CTRL, false);
	GPIO_SetState(GPIO_V_SHORT_PAU, false);
	GPIO_SetState(GPIO_V_SHORT_OUT, false);

	GPIO_SetState(GPIO_C_C_START, false);
	GPIO_SetState(GPIO_C_ENABLE, false);
	GPIO_SetState(GPIO_C_DIAG_CTRL, true);
	GPIO_SetState(GPIO_C_EXT_DAC_CS, true);
	GPIO_SetState(GPIO_C_EXT_DAC_LDAC, true);

	GPIO_SetState(GPIO_SYNC_TOCUHP, false);
	GPIO_SetState(GPIO_SYNC_OSC, false);
	GPIO_SetState(GPIO_SYNC_PAU, false);

	GPIO_SetState(GPIO_OUT_MULTIPLEX, false);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_UART1_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART1_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_SPI1_CLK, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_SPI1_DAT, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_RX, AltFn_9);

	// Аналоговые выходы
	GPIO_InitAnalog(GPIO_C_C_SET);
	GPIO_InitAnalog(GPIO_V_V_SET);

}
//------------------------------------------------

void INITCFG_ConfigUART()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
}
//------------------------------------------------

void INITCFG_ConfigCAN()
{
	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, false);
	NCAN_FIFOInterrupt(true);
	NCAN_FilterInit(0, CAN_SLAVE_FILTER_ID, CAN_SLAVE_NID_MASK);
	NCAN_FilterInit(1, CAN_MASTER_FILTER_ID, CAN_MASTER_NID_MASK);
}
//------------------------------------

void INITCFG_ConfigTimer7()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 3, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void INITCFG_ConfigTimer6()
{
	TIM_Clock_En(TIM_6);
	TIM_Config(TIM6, SYSCLK, TIMER6_uS);
	TIM_DMA(TIM6, DMAEN);
	TIM_MasterMode(TIM6, MMS_UPDATE);
}
//------------------------------------------------

void INITCFG_ConfigTimer15()
{
	TIM_Clock_En(TIM_15);
	TIM_Config(TIM15, SYSCLK, TIMER15_uS);
	TIM_Interupt(TIM15, 1, true);
}
//------------------------------------------------

void INITCFG_ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);
	RCC_ADC_Clk_EN(ADC_34_ClkEN);

	// ADC1
	ADC_Calibration(ADC1);
	ADC_SoftTrigConfig(ADC1);

	ADC_TrigConfig(ADC1, ADC12_TIM6_TRGO, RISE);
	ADC_ChannelSeqReset(ADC1);

	ADC_ChannelSet_Sequence(ADC1, ADC1_C_C_SEN_CHANNEL, 1);
	ADC_ChannelSet_Sequence(ADC1, ADC1_C_V_SEN_CHANNEL, 1);
	ADC_ChannelSeqLen(ADC1, 1);
	ADC_DMAConfig(ADC1);
	ADC_Enable(ADC1);
	ADC_DMAEnable(ADC1, true);

	// ADC3
	ADC_Calibration(ADC3);
	ADC_SoftTrigConfig(ADC3);
	ADC_DMAConfig(ADC3);
	ADC_Enable(ADC3);
	ADC_DMAEnable(ADC3, false);
}
//------------------------------------------------

void INITCFG_ConfigDMA()
{
	DMA_Clk_Enable(DMA1_ClkEN);
	// DMA для АЦП тока затвора
	DMA_Reset(DMA_ADC_C_SEN_CHANNEL);
	DMA_Interrupt(DMA_ADC_C_SEN_CHANNEL, DMA_TRANSFER_COMPLETE, 0, true);
	DMAChannelX_DataConfig(DMA_ADC_C_SEN_CHANNEL, (uint32_t)MEASURE_C_SenRaw, (uint32_t)(&ADC1->DR),
	C_VALUES_x_SIZE);
	DMAChannelX_Config(DMA_ADC_C_SEN_CHANNEL, DMA_MEM2MEM_DIS, DMA_LvlPriority_LOW, DMA_MSIZE_16BIT, DMA_PSIZE_16BIT,
	DMA_MINC_EN, DMA_PINC_DIS, DMA_CIRCMODE_DIS, DMA_READ_FROM_PERIPH);
	DMA_ChannelEnable(DMA_ADC_C_SEN_CHANNEL, true);
}
//------------------------------------------------

void INITCFG_ConfigExtInterrupt()
{
	EXTI_Config(EXTI_PB, EXTI_2, FALL_TRIG, 1);
	EXTI_EnableInterrupt(EXTI2_TSC_IRQn, 1, true);

	EXTI_Config(EXTI_PB, EXTI_15, FALL_TRIG, 2);
	EXTI_EnableInterrupt(EXTI15_10_IRQn, 2, true);
}
//------------------------------------------------

void INITCFG_ConfigSPI()
{
	SPI_Init(SPI1, SPI1_BAUDRATE_BITS, SPI1_LSB_FIRST);
	SPI_InvertClockPolarity(SPI1, true);
}
//------------------------------------------------
