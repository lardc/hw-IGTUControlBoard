// Include
//
#include "Global.h"
#include "Controller.h"
#include "Interrupts.h"
#include "SysConfig.h"
#include "BoardConfig.h"
#include "BCCIxParams.h"

// Forward functions
//
void ConfigSysClk();
void ConfigGPIO();
void ConfigUART();
void ConfigTimer2();
void ConfigWatchDog();

// Functions
//
int main()
{
	// Set request flag if firmware update is required
	if(*ProgramAddressStart == 0xFFFFFFFF || BOOT_LOADER_VARIABLE == BOOT_LOADER_REQUEST)
		WaitForFWUpload = TRUE;
	
	// Init peripherals
	ConfigSysClk();
	ConfigGPIO();
	ConfigUART();
	ConfigTimer2();
	ConfigWatchDog();
	
	// Init controller
	CONTROL_Init();
	
	// Infinite cycle
	while(true)
		CONTROL_Idle();
	
	return 0;
}
//--------------------------------------------

void ConfigSysClk()
{
	RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//--------------------------------------------

void ConfigGPIO()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	
	//Выходы
	GPIO_Config(LED_BLINK_PORT, LED_BLINK_PIN, Output, PushPull, HighSpeed, NoPull);
	//
	GPIO_Config(EXT_DAC_PORT, EXT_DAC_CS, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(EXT_DAC_PORT, EXT_DAC_CS);
	GPIO_Config(EXT_DAC_PORT, EXT_DAC_LDAC, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(EXT_DAC_PORT, EXT_DAC_LDAC);
	GPIO_Config(EXT_DAC_PORT, EXT_DAC_DATA, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(EXT_DAC_PORT, EXT_DAC_DATA);
	GPIO_Config(EXT_DAC_CLK_PORT, EXT_DAC_CLK, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(EXT_DAC_PORT, EXT_DAC_CLK);
	//
	GPIO_Config(A_SET_PORT, A_SET_I, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(A_SET_PORT, A_SET_I);
	GPIO_Config(A_SET_PORT, A_SET_U, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(A_SET_PORT, A_SET_U);
	//
	GPIO_Config(SHORT_OUT_PORT, SHORT_OUT_PIN, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Rst(SHORT_OUT_PORT, SHORT_OUT_PIN);
	//
	GPIO_Config(I_START_PORT, I_START_PIN, Output, PushPull, HighSpeed, NoPull);
	GPIO_Bit_Set(I_START_PORT, I_START_PIN);
	
	//Альтернативные функции портов
	GPIO_Config(GPIOA, Pin_9, AltFn, PushPull, HighSpeed, NoPull); //PA9(USART1 TX)
	GPIO_AltFn(GPIOA, Pin_9, AltFn_7);
	
	GPIO_Config(GPIOA, Pin_10, AltFn, PushPull, HighSpeed, NoPull); //PA10(USART1 RX)
	GPIO_AltFn(GPIOA, Pin_10, AltFn_7);
	
}
//--------------------------------------------

void ConfigUART()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
}
//--------------------------------------------

void ConfigTimer2()
{
	TIM_Clock_En(TIM_2);
	TIM_Config(TIM2, SYSCLK, TIMER2_uS);
	TIM_Interupt(TIM2, 0, true);
	TIM_Start(TIM2);
}
//--------------------------------------------

void ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureFastUpdate();
}
//--------------------------------------------
