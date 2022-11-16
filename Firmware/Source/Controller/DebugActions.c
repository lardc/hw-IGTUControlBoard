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
void DBGACT_V_VSet()
{
	LL_V_VSetDAC(CU_V_VToDAC((float)DataTable[REG_DBG]));
}
//-----------------------------

void DBGACT_V_ShortOut()
{
	DataTable[REG_DBG] == 0 ? LL_V_ShortOut(false) : LL_V_ShortOut(true);
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
	ExDAC_C_VCutoff(DataTable[REG_DBG]);
}
//-----------------------------

void DBGACT_C_VNegativeSet()
{
	ExDAC_C_VNegative(DataTable[REG_DBG]);
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


//
bool DIAG_HandleDebugAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch (ActionID)
	{
		case ACT_DBG_V_V_SET:
			{
				if(CONTROL_State == DS_None)
					DBGACT_V_VSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_V_SHORT:
			{
				if(CONTROL_State == DS_None)
					DBGACT_V_ShortOut();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_V_V_SEN:
			{
				if(CONTROL_State == DS_None)
					DBGACT_V_VSen();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_V_C_SEN:
			{
				if(CONTROL_State == DS_None)
					DBGACT_V_CSen();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;
		//
		case ACT_DBG_C_C_SET:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_CSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_C_START:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_CStart();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_C_C_SEN:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_CSen();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_C_V_CUTOFF:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_VCutoffSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_C_V_NEGATIVE:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_VNegativeSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_C_TEST_PULSE:
			{
				if(CONTROL_State == DS_None)
					DBGACT_C_TestPulse();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		default:
			return false;
	}

	return true;
}
