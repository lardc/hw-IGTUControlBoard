#ifndef __DIAGNOSTIC_H
#define __DIAGNOSTIC_H

// Include
#include "stdinc.h"

#define DIAG_SUCCESS				1
#define DIAG_FAULT					0

//V
#define DIAG_REG_VGS_C_TRIG			20
#define DIAG_REG_VGS_T_V_CONSTANT	5
#define DIAG_REG_VGS_T_V_FRONT		5
#define DIAG_REG_VGS_V_MAX			5000

#define DIAG_VGS_THRESHOLD_MIN		1000
#define DIAG_VGS_THRESHOLD_MAX		6000

//C
#define DIAG_REG_QG_V_CUTOFF		15000
#define DIAG_REG_QG_V_NEGATIVE		0
#define DIAG_REG_QG_C_SET			20
#define DIAG_REG_QG_T_CURRENT		200
#define DIAG_REG_QG_C_POWER			0
#define DIAG_REG_QG_V_POWER			0
#define DIAG_REG_QG_C_THRESHOLD		30

#define DIAG_QG_THRESHOLD_MIN		300
#define DIAG_QG_THRESHOLD_MAX		1000

// Functions
bool DIAG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError);

#endif // __DIAGNOSTIC_H
