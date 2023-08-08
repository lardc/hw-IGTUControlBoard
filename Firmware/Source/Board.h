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
// INDICATION
GPIO_PortPinSettingMacro GPIO_INDICATION 		= {GPIOC, Pin_15};
// V
GPIO_PortPinSettingMacro GPIO_V_CURR_K1 		= {GPIOA, Pin_6};
GPIO_PortPinSettingMacro GPIO_V_CURR_K2 		= {GPIOB, Pin_10};
GPIO_PortPinSettingMacro GPIO_V_DIAG_CTRL 		= {GPIOB, Pin_0};
GPIO_PortPinSettingMacro GPIO_V_SHORT_PAU 		= {GPIOB, Pin_5};
GPIO_PortPinSettingMacro GPIO_V_SHORT_OUT 		= {GPIOB, Pin_6};
GPIO_PortPinSettingMacro GPIO_V_LOW_CURRENT 	= {GPIOB, Pin_8};
GPIO_PortPinSettingMacro GPIO_V_HIGH_CURRENT 	= {GPIOB, Pin_9};
// C
GPIO_PortPinSettingMacro GPIO_C_C_START 		= {GPIOB, Pin_7};
GPIO_PortPinSettingMacro GPIO_C_ENABLE 			= {GPIOB, Pin_12};
GPIO_PortPinSettingMacro GPIO_C_DIAG_CTRL 		= {GPIOB, Pin_14};
GPIO_PortPinSettingMacro GPIO_QG_PORTECTION		= {GPIOC, Pin_13};
// C EXT DAC
GPIO_PortPinSettingMacro GPIO_C_EXT_DAC_CS 		= {GPIOA, Pin_15};
GPIO_PortPinSettingMacro GPIO_C_EXT_DAC_LDAC	= {GPIOA, Pin_8};
// SYNC
GPIO_PortPinSettingMacro GPIO_SYNC_TOCUHP 		= {GPIOB, Pin_4};
GPIO_PortPinSettingMacro GPIO_SYNC_OSC 			= {GPIOB, Pin_11};
GPIO_PortPinSettingMacro GPIO_SYNC_PAU 			= {GPIOB, Pin_13};
// OUTPUT
GPIO_PortPinSettingMacro GPIO_OUT_MULTIPLEX 	= {GPIOC, Pin_14};

// Определения для входных портов
GPIO_PortPinSettingMacro GPIO_FB_PAU 			= {GPIOB, Pin_2};
GPIO_PortPinSettingMacro GPIO_SYNC_SAFETY 		= {GPIOB, Pin_15};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_UART1_TX 		= {GPIOA, Pin_9};
GPIO_PortPinSettingMacro GPIO_ALT_UART1_RX 		= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_TX  		= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_CAN1_RX  		= {GPIOA, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_CLK 		= {GPIOB, Pin_3};
GPIO_PortPinSettingMacro GPIO_ALT_SPI1_DAT 		= {GPIOA, Pin_7};

// Определения для аналоговых портов
GPIO_PortPinSettingMacro GPIO_C_C_SET 			= {GPIOA, Pin_4};
GPIO_PortPinSettingMacro GPIO_V_V_SET 			= {GPIOA, Pin_5};

#endif // __BOARD_H
