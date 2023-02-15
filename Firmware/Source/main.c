#include "Controller.h"
#include "InitConfig.h"
#include "SysConfig.h"

int main()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
	
	// Настройка системной частоты тактирования
	INITCFG_ConfigSystemClock();
	
	// Настройка портов
	INITCFG_ConfigIO();
	
	// Настройка UART
	INITCFG_ConfigUART();
	
	// Настройка CAN
	INITCFG_ConfigCAN();

	// Настройка SPI внешнего ЦАП
	INITCFG_ConfigSPI();

	// Настройка системного счетчика
	INITCFG_ConfigTimer7();
	
	// Настройка таймера для Регулятора
	INITCFG_ConfigTimer15();

	// Настройка таймера для триггера ЦАП
	INITCFG_ConfigTimer4();

	// Настройка АЦП
	INITCFG_ConfigADC();

	// Настройка DMA
	INITCFG_ConfigDMA();

	// Настройка DAC
	INITCFG_ConfigDAC();

	// Настройка сторожевого таймера
	INITCFG_ConfigWatchDog();
	
	// Настройка внешнего прерывания
	INITCFG_ConfigExtInterrupt();

	// Инициализация логики контроллера
	CONTROL_Init();
	
	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
