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
#include "ExternalDAC.h"

// Functions
//
// Источник напряжения
//
void DBGACT_UUSet()
{
	LL_UUSetDAC(CU_UUToDAC((float)DataTable[REG_DBG]));
}
//-----------------------------

void DBGACT_UShortOut()
{
	DataTable[REG_DBG] == 0 ? LL_UShortOut(false) : LL_UShortOut(true);
}
//-----------------------------

void DBGACT_UUSen()
{
	DataTable[REG_DBG] = (Int16U)CU_UADCUToX(MEASURE_UUSen());
}
//-----------------------------

void DBGACT_UISen()
{
	DataTable[REG_DBG] = (Int16U)CU_UADCIToX(MEASURE_UISen());
}
//-----------------------------

// Источник тока
//
void DBGACT_IISet()
{
	LL_IISetDAC(CU_IIToDAC((float)DataTable[REG_DBG]));
	DELAY_US(20);
	LL_IISetDAC(0);
}
//-----------------------------

void DBGACT_IStart()
{
	DataTable[REG_DBG] == 1 ? LL_IStart(false) : LL_IStart(true);
}
//-----------------------------

void DBGACT_IIGate()
{
	DataTable[REG_DBG] = (Int16U)CU_IADCIToX(MEASURE_IIGate());
}
//-----------------------------

void DBGACT_UUCutoffSet()
{
	ExDAC_IUCutoff(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_UUNegativeSet()
{
	ExDAC_IUNegative(DataTable[REG_DBG]);
}

void DBGACT_ITestPulse()
{
	LL_IISetDAC(CU_IIToDAC((float)DataTable[REG_DBG]));
	DELAY_US(5);
	LL_IStart(false);
	DELAY_US(20);
	LL_IStart(true);
	LL_IISetDAC(0);
}
//-----------------------------
