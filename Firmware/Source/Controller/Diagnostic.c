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
		case ACT_DIAG_VGS:
			break;

		case ACT_DIAG_QG:
			break;

		case ACT_DIAG_IGES:
			break;

		default:
			return false;
	}

	return true;
}



