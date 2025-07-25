#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"

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

void INITCFG_IO()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	
	// Выходы
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_SPI_SS);
	GPIO_SetState(GPIO_SPI_SS, true);
	GPIO_InitPushPullOutput(GPIO_SPI_LDAC);
	GPIO_SetState(GPIO_SPI_LDAC, true);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_CAN_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_UART_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART_TX, AltFn_7);

	GPIO_InitAltFunction(GPIO_ALT_SPI_CLCK, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_SPI_MOSI, AltFn_5);
}
//------------------------------------------------

void INITCFG_UART()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
	NVIC_SetPriority(EXTI15_10_IRQn, USART1_INTERRUPT_PRIORITY);
}
//------------------------------------------------

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
}
//------------------------------------------------

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
	SPI1->CR1 |= BIT1;
}
//------------------------------------------------
