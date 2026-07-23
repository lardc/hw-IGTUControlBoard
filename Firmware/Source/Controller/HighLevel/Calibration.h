#ifndef CONTROLLER_CALIBRATION_H_
#define CONTROLLER_CALIBRATION_H_

// Include
//
#include "Board.h"
#include "stdinc.h"
#include "Measurement.h"
#include "Controller.h"

// Functions prototypes
//
void CAL_V_Prepare();
void CAL_I_Prepare();
void CAL_V_CalProcess();
void CAL_I_CalProcess();

#endif /* CONTROLLER_CALIBRATION_H_ */
