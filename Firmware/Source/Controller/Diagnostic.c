// Header
#include "Diagnostic.h"

// Includes
#include "DataTable.h"
#include "LowLevel.h"
#include "Controller.h"
#include "DebugActions.h"


// Functions
bool DIAG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch (ActionID)
	{
		case ACT_DBG_U_U_SET:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UUSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_U_SHORT:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UShortOut();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_U_U_SEN:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UUSen();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_U_I_SEN:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UISen();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		//
		case ACT_DBG_I_I_SET:
			{
				if(CONTROL_State == DS_None)
					DBGACT_IISet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_I_START:
			{
				if(CONTROL_State == DS_None)
					DBGACT_IStart();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_I_I_GATE:
			{
				if(CONTROL_State == DS_None)
					DBGACT_IIGate();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_I_U_CUTOFF:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UUCutoffSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_I_U_NEGATIVE:
			{
				if(CONTROL_State == DS_None)
					DBGACT_UUNegativeSet();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_I_TEST_PULSE:
			{
				if(CONTROL_State == DS_None)
					DBGACT_ITestPulse();
				else
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;

		default:
			return false;
	}

	return true;
}
