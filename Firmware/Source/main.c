#include "Controller.h"
#include "InitConfig.h"
#include "SysConfig.h"

int main()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
	
	// Настройка системной частоты тактирования
	INITCFG_SysClk();
	
	// Настройка портов
	INITCFG_IO();
	
	// Настройка внешних прерываний
	INITCFG_EI();
	
	// Настройка UART
	INITCFG_UART();
	
	// Настройка системного счетчика
	INITCFG_Timer7();
	
	// Настройка таймера таймаута длительности сихронизации
	INITCFG_Timer3();

	// Настройка АЦП
	INITCFG_ADC();
	
	// Настройка сторожевого таймера
	INITCFG_WatchDog();
	
	// Инициализация логики контроллера
	CONTROL_Init();
	
	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
