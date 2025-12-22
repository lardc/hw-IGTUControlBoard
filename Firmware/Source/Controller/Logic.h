#ifndef CONTROLLER_LOGIC_H_
#define CONTROLLER_LOGIC_H_

// Definitions
//
#define INIT_48V_TIMER			10	// Время для выставления 48v на плате, в мс
#define REGLTR_TIMER			10	// Время для выхода регулятора на рабочее напряжение, в мс

// Includes
//
#include "stdinc.h"

// Variables
//
extern Int16U LOGIC_ChannelNumber;
//Functions
//
void LOGIC_HandleMeasurement();

#endif // CONTROLLER_LOGIC_H_
