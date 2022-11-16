// -----------------------------------------
// Board parameters
// ----------------------------------------

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Board includes
//
#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwSCI.h"
#include "ZwTIM.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"

// Definitions
//
// Blinking LED settings
#define LED_BLINK_PORT		GPIOC
#define LED_BLINK_PIN		Pin_13
// External DAC SPI
#define C_EXT_DAC_PORT		GPIOA
#define C_EXT_DAC_CLK_PORT	GPIOB
#define C_EXT_DAC_CS		Pin_15
#define C_EXT_DAC_LDAC		Pin_8
#define C_EXT_DAC_DATA		Pin_7
#define C_EXT_DAC_CLK		Pin_3
// Analog set
#define C_A_SET_PORT		GPIOA
#define C_C_SET_PIN			Pin_4
#define C_V_SET_PIN			Pin_5
// Short output circuit
#define V_SHORT_OUT_PORT	GPIOB
#define V_SHORT_OUT_PIN		Pin_6
// C Enable
#define C_ENABLE_PORT		GPIOB
#define C_ENABLE_PIN		Pin_6
// C start control
#define C_C_START_PORT		GPIOB
#define C_C_START_PIN		Pin_0

#endif // __BOARD_CONFIG_H
