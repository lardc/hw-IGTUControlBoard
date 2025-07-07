#include "Controller.h"
#include "InitConfig.h"
#include "SysConfig.h"

int main()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
	
	INITCFG_SysClk();
	INITCFG_IO();
	INITCFG_UART();
	INITCFG_Timer7();
	INITCFG_Timer3();
	INITCFG_SPI();
	INITCFG_WatchDog();
	
	// Инициализация логики контроллера
	CONTROL_Init();
	
	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
