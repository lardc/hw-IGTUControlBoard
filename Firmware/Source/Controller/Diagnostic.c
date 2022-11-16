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
			{
				/*if(CONTROL_State == DS_None)
				else
					*pUserError = ERR_OPERATION_BLOCKED;*/
			}
			break;

		case ACT_DIAG_QG:
			{
				/*if(CONTROL_State == DS_None)
				else
					*pUserError = ERR_OPERATION_BLOCKED;*/
			}
			break;

		case ACT_DIAG_IGES:
			{
				/*if(CONTROL_State == DS_None)
					//
				else
					*pUserError = ERR_OPERATION_BLOCKED;*/
			}
			break;

		default:
			return false;
	}

	return true;
}



