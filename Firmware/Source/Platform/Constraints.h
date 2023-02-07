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

#define DAC_OUTPUT_LIM_MIN			0
#define DAC_OUTPUT_LIM_MAX			4095
#define DAC_OUTPUT_LIM_DEF			4095
//
#define DAC_OFFSET_MIN				0
#define DAC_OFFSET_MAX				4095
#define DAC_OFFSET_DEF				0
//
#define V_C_SENS_THRESHOLD_MAX		20
//
#define PAU_CAN_ID_DEF				101
#define TOCUHP_CAN_ID_DEF			102
//
#define COEF_P0_MIN					INT16S_MIN
#define COEF_P0_MAX					INT16S_MAX
#define COEF_P0_DEF					0
//
#define COEF_P1_MIN					1
#define COEF_P1_MAX					INT16U_MAX
#define COEF_P1_DEF					1000
//
#define COEF_P2_MIN					INT16S_MIN
#define COEF_P2_MAX					INT16S_MAX
#define COEF_P2_DEF					0
//
#define COEF_K_MIN					0
#define COEF_K_MAX					INT16U_MAX
#define COEF_C_C_SEN_K_DEF			0.122		// [мА / код]
#define COEF_V_C_SEN_K_DEF			0.141		// [мА / код]
#define COEF_V_V_SEN_K_DEF			7.324		// [мВ / код]
#define COEF_C_C_K_DEF				6.827		// [код / мА]
#define COEF_V_V_K_DEF				0.204		// [код / мВ]
#define COEF_C_V_CUTOFF_K_DEF		0.162		// [код / мВ]
#define COEF_C_V_NEGATIVE_K_DEF		0.201		// [код / мВ]
#define COEF_POT_SEN_K_DEF			3.002		// [мВ / код]
#define COEF_C_V_SEN_K_DEF			13.47		// [мВ / код]
//
#define OFFSET_MIN					0
#define OFFSET_MAX					INT16U_MAX
#define OFFSET_DEF					0
//
#define REGULATOR_KP_MIN			0
#define REGULATOR_KP_MAX			INT16U_MAX
#define REGULATOR_KP_DEF			0.001
//
#define REGULATOR_KI_MIN			0
#define REGULATOR_KI_MAX			INT16U_MAX
#define REGULATOR_KI_DEF			0
//
#define PAU_SNC_DELAY_DEF			5
//
#define TOCUHP_RES_PER_BIT_MIN		1			// [Ом]
#define TOCUHP_RES_PER_BIT_MAX		10000		// [Ом]
#define TOCUHP_RES_PER_BIT_DEF		1000		// [Ом]
//
// VGS
#define VGS_C_TRIG_MIN				1			// [мА]
#define VGS_C_TRIG_MAX				300			// [мА]
//
#define VGS_T_V_CONSTANT_MIN		1			// [мс]
#define VGS_T_V_CONSTANT_MAX		20			// [мс]
//
#define VGS_T_V_FRONT_MIN			1			// [мс]
#define VGS_T_V_FRONT_MAX			20			// [мс]
//
#define VGS_V_DEST_MIN				100			// [мВ]
#define VGS_V_DEST_MAX				20000		// [мВ]
//
// QG
#define QG_V_CUTOFF_MIN				0			// [мВ]
#define QG_V_CUTOFF_MAX				20000		// [мВ]
#define QG_V_CUTOFF_DEF				15000		// [мВ]
//
#define QG_V_NEGATIVE_MIN			0			// [мВ]
#define QG_V_NEGATIVE_MAX			20000		// [мВ]
#define QG_V_NEGATIVE_DEF			8000		// [мВ]
//
#define QG_C_SET_MIN				20			// [мА]
#define QG_C_SET_MAX				500			// [мА]
//
#define QG_T_CURRENT_MIN			20			// [мс]
#define QG_T_CURRENT_MAX			500			// [мс]
//
#define QG_C_POWER_SET_MIN			0			// [A]
#define QG_C_POWER_SET_MAX			1000		// [A]
//
#define QG_V_POWER_SET_MIN			0			// [В]
#define QG_V_POWER_SET_MAX			1800		// [В]
//
#define QG_C_THRESHOLD_MIN			10			// [%]
#define QG_C_THRESHOLD_MAX			100			// [%]
#define QG_C_THRESHOLD_DEF			50			// [%]
// IGES
#define IGES_V_CONSTANT_MIN			0			// [мВ]
#define IGES_V_CONSTANT_MAX			30000		// [мВ]
//
#define IGES_T_V_CONSTANT_MIN		10			// [мс]
#define IGES_T_V_CONSTANT_MAX		100			// [мс]
//
#define IGES_T_V_FRONT_MIN			1			// [мс]
#define IGES_T_V_FRONT_MAX			10			// [мс]
//

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
