// Header
#include "Diagnostic.h"

// Includes
#include "DataTable.h"
#include "LowLevel.h"
#include "Controller.h"
#include "DebugActions.h"
#include "Delay.h"

// Functions
bool DIAG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch(ActionID)
	{
		case ACT_DBG_SYNC_PAU:
			DBGACT_SyncPAU();
			break;

		case ACT_DBG_SYNC_TOCUHP:
			DBGACT_SyncTOCUHP();
			break;

		case ACT_DBG_SYNC_OSC:
			DBGACT_SyncOSC();
			break;

		case ACT_DBG_EXT_IND:
			DBGACT_BlinkExtIndication();
			break;

		case ACT_DBG_SWITCH_MUX:
			DBGACT_SwitchMUX();
			break;

		case ACT_DBG_PAU_SHORT:
			DBGACT_ShortPAU();
			break;

		case ACT_DBG_V_SHORT:
			DBGACT_V_ShortOut();
			break;

		case ACT_DBG_V_V_SET:
			DBGACT_V_VSet();
			break;

		case ACT_DBG_C_LIM_LOW_TEST:
			DBGACT_V_TestClimLow();
			break;

		case ACT_DBG_C_LIM_HIGH_TEST:
			DBGACT_V_TestClimHigh();
			break;

		case ACT_DBG_C_V_CUTOFF:
			DBGACT_C_VCutoffSet();
			break;

		case ACT_DBG_C_V_NEGATIVE:
			DBGACT_C_VNegativeSet();
			break;

		case ACT_DBG_C_TEST_PULSE:
			DBGACT_C_TestPulse();
			break;

		case ACT_DBG_SWITCH_TO_DIAG:
			DBGACT_SwitchToDIAG();
			break;

		default:
			return false;
	}

	return true;
}
