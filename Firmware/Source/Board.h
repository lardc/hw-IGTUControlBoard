#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f30x.h"

#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwTIM.h"
#include "ZwDAC.h"
#include "ZwDMA.h"
#include "ZwADC.h"
#include "ZwEXTI.h"
#include "ZwSCI.h"
#include "ZwSPI.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"


// Определения для выходных портов
GPIO_PortPinSettingMacro GPIO_EXT_DAC_CS		= {GPIOA, Pin_1};
GPIO_PortPinSettingMacro GPIO_EXT_DAC_LDAC		= {GPIOA, Pin_2};
//
GPIO_PortPinSettingMacro GPIO_LED				= {GPIOB, Pin_7};
GPIO_PortPinSettingMacro GPIO_SHORT_OUT			= {GPIOB, Pin_6};
GPIO_PortPinSettingMacro GPIO_I_START			= {GPIOB, Pin_0};

// Определения для входных портов
GPIO_PortPinSettingMacro GPIO_I_COMP			= {GPIOA, Pin_15};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_UART1_TX		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART1_RX		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_CLK		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_DAT		= {GPIOA, Pin_7};

#endif // __BOARD_H
