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
#include "ZwIWDG.h"
#include "ZwNFLASH.h"

// Определения для выходных портов
GPIO_PortPinSettingMacro GPIO_LED			= {GPIOA, Pin_8};
GPIO_PortPinSettingMacro GPIO_SPI_SS		= {GPIOA, Pin_15};
//
GPIO_PortPinSettingMacro GPIO_LED_EXT		= {GPIOB, Pin_4};
GPIO_PortPinSettingMacro GPIO_SPI_OE		= {GPIOB, Pin_7};
GPIO_PortPinSettingMacro GPIO_SPI_CLK		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_SPI_DAT		= {GPIOB, Pin_5};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_CAN_RX	= {GPIOA, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_CAN_TX	= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_UART_RX	= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_UART_TX	= {GPIOA, Pin_9};

#endif // __BOARD_H
