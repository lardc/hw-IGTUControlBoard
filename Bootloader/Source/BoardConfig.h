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
#define LED_BLINK_PORT		GPIOB
#define LED_BLINK_PIN		Pin_7
// External DAC SPI
#define EXT_DAC_PORT		GPIOA
#define EXT_DAC_CLK_PORT	GPIOB
#define EXT_DAC_CS			Pin_1
#define EXT_DAC_LDAC		Pin_2
#define EXT_DAC_DATA		Pin_7
#define EXT_DAC_CLK			Pin_3
// Analog set
#define A_SET_PORT			GPIOA
#define A_SET_I				Pin_4
#define A_SET_U				Pin_5
// Short output circuit
#define SHORT_OUT_PORT		GPIOB
#define SHORT_OUT_PIN		Pin_6
// I start control
#define I_START_PORT		GPIOB
#define I_START_PIN			Pin_0

#endif // __BOARD_CONFIG_H
