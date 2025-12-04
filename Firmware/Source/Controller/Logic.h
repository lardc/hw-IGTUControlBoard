#ifndef CONTROLLER_LOGIC_H_
#define CONTROLLER_LOGIC_H_

// Definitions
//
#define INIT_48V_TIMER			10 // Время для выставления 48v на плате, в мс
#define SW_CURRENT_CH_TIMER		1	// Время для переключения реле тока , в мс
#define REGLTR_TIMER			10	// Время для выхода регулятора на рабочее напряжение, в мс

// Includes
//
#include "stdinc.h"

//Functions
//
void LOGIC_HandleMeasurement();

#endif // CONTROLLER_LOGIC_H_
