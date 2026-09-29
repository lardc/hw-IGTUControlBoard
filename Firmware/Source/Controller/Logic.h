#ifndef CONTROLLER_LOGIC_H_
#define CONTROLLER_LOGIC_H_

// Definitions
//

// Includes
//
#include "stdinc.h"

// Variables
//
extern Int16U LOGIC_ChannelNumber;
//Functions
//
void LOGIC_HandleMeasurement();
void LOGIC_SetAutoBothSelfTest(Boolean Enable);

#endif // CONTROLLER_LOGIC_H_
