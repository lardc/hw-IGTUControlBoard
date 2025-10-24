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
GPIO_PortPinSettingMacro GPIO_LED			= {GPIOB, Pin_15};
GPIO_PortPinSettingMacro GPIO_SPI_CS1		= {GPIOA, Pin_15};
GPIO_PortPinSettingMacro GPIO_SPI_CS2		= {GPIOB, Pin_12};
GPIO_PortPinSettingMacro GPIO_PULSE			= {GPIOA, Pin_2};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_CAN_RX	= {GPIOA, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_CAN_TX	= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_UART_RX	= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_UART_TX	= {GPIOA, Pin_9};

GPIO_PortPinSettingMacro GPIO_ALT_SPI_CLCK	= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI_MOSI	= {GPIOB, Pin_5};

#endif // __BOARD_H
