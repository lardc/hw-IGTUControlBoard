#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
#define COEF_P2_MIN					-1e-3
#define COEF_P2_MAX					1e-3
#define COEF_P2_DEF					0
//
#define COEF_P1_MIN					0.5f
#define COEF_P1_MAX					1.5f
#define COEF_P1_DEF					1
//
#define COEF_P0_MIN					INT16S_MIN
#define COEF_P0_MAX					INT16S_MAX
#define COEF_P0_DEF					0
//
#define COEF_K_MIN					0
#define COEF_K_MAX					INT16U_MAX
//
#define COEF_B_MIN					INT16S_MIN
#define COEF_B_MAX					INT16S_MAX
//
#define COEF_RSH_MIN				0
#define COEF_RSH_MAX				100e+6
//
#define SLEW_RATE_MIN				1.0f
//
#define U_RTH_MIN					110		// В мВ
#define U_RTH_MAX					1100
#define U_RTH_DEF					550
//
#define U_IGES_MIN					94		// В мВ
#define U_IGES_MAX					940
#define U_IGES_DEF					470
//
#define U_ERR_COUNT_MIN				10		// в шт
#define U_ERR_COUNT_MAX				20		// в шт
#define U_ERR_COUNT_DEF				15		// в шт

//
// Types
typedef struct __TableItemConstraint
{
	float Min;
	float Max;
	float Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];

#endif // __CONSTRAINTS_H
