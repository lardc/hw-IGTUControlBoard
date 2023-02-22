﻿#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
#define NO		0
#define YES		1
//
#define V_ADC_TO_V_K_DEF			0.008409
#define V_ADC_TO_V_B_DEF			0.166985
#define V_V_TO_DAC_K_DEF			123.072
#define V_V_TO_DAC_B_DEF			7.385
#define V_ADC_TO_I_R0_K_DEF			6.472063e-003
#define V_ADC_TO_I_R0_B_DEF			3.707259e-002
#define V_ADC_TO_I_R1_K_DEF			5.937186e-002
#define V_ADC_TO_I_R1_B_DEF			1.861462e-001
//
#define COEF_P2_MIN					INT16S_MIN
#define COEF_P2_MAX					INT16S_MAX
#define COEF_P2_DEF					0
//
#define COEF_P1_MIN					INT16S_MIN
#define COEF_P1_MAX					INT16S_MAX
#define COEF_P1_DEF					1
//
#define COEF_P0_MIN					INT16S_MIN
#define COEF_P0_MAX					INT16S_MAX
#define COEF_P0_DEF					0
//
#define COEF_K_MIN					INT16S_MIN
#define COEF_K_MAX					INT16S_MAX
//
#define COEF_B_MIN					INT16S_MIN
#define COEF_B_MAX					INT16S_MAX
//
#define REGULATOR_KP_MIN			INT16S_MIN
#define REGULATOR_KP_MAX			INT16S_MAX
#define REGULATOR_KP_DEF			0.01
//
#define REGULATOR_KI_MIN			INT16S_MIN
#define REGULATOR_KI_MAX			INT16S_MAX
#define REGULATOR_KI_DEF			0.3
//
#define REGULATOR_QI_MAX_DEF		30000
//
#define REGULATOR_ERR_MIN			1
#define REGULATOR_ERR_MAX			10
#define REGULATOR_ERR_DEF			5
//
#define REGULATOR_FE_CNT_MIN		0
#define REGULATOR_FE_CNT_MAX		10
#define REGULATOR_FE_CNT_DEF		5
//
#define VGS_FAST_RATE_MIN			1
#define VGS_FAST_RATE_MAX			10
#define VGS_FAST_RATE_DEF			5
//
#define VGS_SLOW_RATE_MIN			1
#define VGS_SLOW_RATE_MAX			10
#define VGS_SLOW_RATE_DEF			5
//
#define VGS_dI_TRIG_MIN				1
#define VGS_dI_TRIG_MAX				10
#define VGS_dI_TRIG_DEF				5
//
#define VGS_I_THRESHOLD_MIN			10
#define VGS_I_THRESHOLD_MAX			24
#define VGS_I_THRESHOLD_DEF			20
//
#define IGES_V_RATE_MIN				1
#define IGES_V_RATE_MAX				10
#define IGES_V_RATE_DEF				5
//
#define IGES_PLATE_TIME_MIN			100
#define IGES_PLATE_TIME_MAX			10000
#define IGES_PLATE_TIME_DEF			5000
//
#define PAU_CAN_ID_DEF				120
#define TOCUHP_CAN_ID_DEF			130
#define TOCUHP_RES_PER_BIT_DEF		200
//
#define DAC_OUTPUT_LIM_MIN			0
#define DAC_OUTPUT_LIM_MAX			4095
#define DAC_OUTPUT_LIM_DEF			4095
//
#define ST_CHECK_ERROR_MIN			1
#define ST_CHECK_ERROR_MAX			10
#define ST_CHECK_ERROR_DEF			5
//
// VGS
#define VGS_I_TRIG_MIN				1			// (мА)
#define VGS_I_TRIG_MAX				300			// (мА)
#define VGS_I_TRIG_DEF				300			// (мА)
//
#define VGS_V_DEST_MIN				100			// (мВ)
#define VGS_V_DEST_MAX				20000		// (мВ)
#define VGS_V_DEST_DEF				20000		// (мВ)
//
#define QG_V_CUTOFF_MIN				5000		//
#define QG_V_CUTOFF_MAX				20000		//
#define QG_V_CUTOFF_DEF				20000		//
//
#define QG_V_NEGATIVE_MIN			5000
#define QG_V_NEGATIVE_MAX			20000
#define QG_V_NEGATIVE_DEF			5000
//
#define QG_I_MIN					20
#define QG_I_MAX					500
#define QG_I_DEF					500
//
#define QG_I_DURATION_MIN			20
#define QG_I_DURATION_MAX			500
#define QG_I_DURATION_DEF			500
//
#define QG_I_POWER_MIN				50
#define QG_I_POWER_MAX				1000
#define QG_I_POWER_DEF				1000
//
#define QG_V_POWER_MIN				100
#define QG_V_POWER_MAX				1500
#define QG_V_POWER_DEF				1000
//
#define IGES_V_MIN					5000
#define IGES_V_MAX					30000
#define IGES_V_DEF					30000


// Types
typedef struct __TableItemConstraint
{
	float Min;
	float Max;
	float Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint Constraint[];

#endif // __CONSTRAINTS_H
