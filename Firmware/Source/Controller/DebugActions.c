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
#include "Global.h"

// Definitions
//
#define DBG_DAC_TEST_VALUE		3000

// Functions
//
void DBGACT_SyncPAU()
{
	LL_SyncPAU(true);
	DELAY_US(1000);
	LL_SyncPAU(false);
}
//-----------------------------

void DBGACT_SyncTOCUHP()
{
	LL_SyncTOCUHP(true);
	DELAY_US(1000);
	LL_SyncTOCUHP(false);
}
//-----------------------------

void DBGACT_SyncOSC()
{
	LL_SyncOSC(true);
	DELAY_US(1000);
	LL_SyncOSC(false);
}
//-----------------------------

void DBGACT_BlinkExtIndication()
{
	LL_Indication(true);
	DELAY_MS(1000);
	LL_Indication(false);
}
//-----------------------------

void DBGACT_SwitchMUX()
{
	(DataTable[REG_DBG]) ? LL_OutMultiplexVoltage() : LL_OutMultiplexCurrent();
}
//-----------------------------

void DBGACT_ShortPAU()
{
	LL_V_ShortPAU(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_V_ShortOut()
{
	LL_V_ShortOut(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_V_VSet()
{
	CU_LoadConvertParams();

	LL_OutMultiplexVoltage();
	LL_V_IlimLowRange();
	LL_V_ShortPAU(true);
	LL_V_ShortOut(false);
	LL_V_Diagnostic(false);
	LL_V_VSetDAC(CU_V_VtoDAC((float)DataTable[REG_DBG]));
}
//-----------------------------

void DBGACT_V_TestClimLow()
{
	LL_OutMultiplexVoltage();
	DELAY_MS(20);

	LL_V_ShortPAU(true);
	LL_V_ShortOut(false);
	LL_V_Diagnostic(false);
	MEASURE_V_SetCurrentRange(0);
	LL_V_IlimLowRange();

	LL_V_VSetDAC(DBG_DAC_TEST_VALUE);
	DELAY_US(1000);
	LL_V_VSetDAC(0);
}
//-----------------------------

void DBGACT_V_TestClimHigh()
{
	LL_OutMultiplexVoltage();
	DELAY_MS(20);

	LL_V_ShortPAU(true);
	LL_V_ShortOut(false);
	LL_V_Diagnostic(false);
	MEASURE_V_SetCurrentRange(V_I_R1_MAX);
	LL_V_IlimHighRange();

	LL_V_VSetDAC(DBG_DAC_TEST_VALUE);
	DELAY_US(1000);
	LL_V_VSetDAC(0);
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

void DBGACT_SwitchToDIAG()
{
	LL_V_Diagnostic(DataTable[REG_DBG]);
	LL_I_Diagnostic(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_Protection()
{
	LL_QgProtection(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_C_TestPulse()
{
	LL_OutMultiplexCurrent();
	LL_I_Diagnostic(false);
	LL_ExDACVNegative(0);
	LL_I_Enable(true);
	LL_QgProtection(false);
	LL_ExDACVCutoff(DBG_DAC_TEST_VALUE);
	DELAY_MS(20);

	LL_I_SetDAC(DataTable[REG_DBG]);
	DELAY_US(10);

	LL_QgProtection(true);
	LL_I_Start(true);
	DELAY_US(20);
	LL_I_Start(false);
	LL_QgProtection(false);

	LL_I_Enable(false);
	LL_I_SetDAC(0);
	LL_ExDACVCutoff(0);
}
//-----------------------------
