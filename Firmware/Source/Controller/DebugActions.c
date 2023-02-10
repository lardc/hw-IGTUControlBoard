// Header
#include "DebugActions.h"

// Include
//
#include "LowLevel.h"
#include "Board.h"
#include "Delay.h"
#include "Controller.h"
#include "DataTable.h"
#include "Controller.h"
#include "Delay.h"
#include "ConvertUtils.h"
#include "Measurement.h"

// Functions
//
// Источник напряжения
//
void DBGACT_V_VSet()
{
	LL_V_VSetDAC(CU_V_VToDAC((float)DataTable[REG_DBG]));
}
//-----------------------------

void DBGACT_V_ShortOut()
{
	LL_V_ShortOut(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_V_VSen()
{
	DataTable[REG_DBG] = (Int16U)CU_V_ADCVToX(MEASURE_V_VSen());
}
//-----------------------------

void DBGACT_V_CSen()
{
	DataTable[REG_DBG] = (Int16U)CU_V_ADCCToX(MEASURE_V_CSen());
}
//-----------------------------

// Источник тока
//
void DBGACT_C_CSet()
{
	LL_C_CSetDAC(CU_C_CToDAC((float)DataTable[REG_DBG]));
	DELAY_US(20);
	LL_C_CSetDAC(0);
}
//-----------------------------

void DBGACT_C_CStart()
{
	DataTable[REG_DBG] == 1 ? LL_C_CStart(false) : LL_C_CStart(true);
}
//-----------------------------

void DBGACT_C_CSen()
{
	DataTable[REG_DBG] = (Int16U)CU_C_ADCCToX(MEASURE_C_CSen());
}
//-----------------------------

void DBGACT_C_VCutoffSet()
{
	LL_ExDACVCutoff(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_C_VNegativeSet()
{
	LL_ExDACVNegative(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_C_TestPulse()
{
	LL_C_CSetDAC(CU_C_CToDAC((float)DataTable[REG_DBG]));
	DELAY_US(5);
	LL_C_CStart(false);
	DELAY_US(20);
	LL_C_CStart(true);
	LL_C_CSetDAC(0);
}
//-----------------------------
