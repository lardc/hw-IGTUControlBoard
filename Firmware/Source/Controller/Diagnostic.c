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
		case ACT_DIAG_V:
			if(CONTROL_State == DS_Ready)
			{
				DataTable[REG_VGS_C_TRIG] = DIAG_REG_VGS_C_TRIG;
				DataTable[REG_VGS_T_V_CONSTANT] = DIAG_REG_VGS_T_V_CONSTANT;
				DataTable[REG_VGS_T_V_FRONT] = DIAG_REG_VGS_T_V_FRONT;
				DataTable[REG_VGS_V_MAX] = DIAG_REG_VGS_V_MAX;
				LL_V_Diagnostic(true);
				DELAY_MS(10);
				CONTROL_SetDeviceState(DS_Selftest, SS_VgsPulse);
				CONTROL_VGS_StartProcess();
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_DIAG_C:
			if(CONTROL_State == DS_Ready)
			{
				DataTable[REG_QG_V_CUTOFF] = DIAG_REG_QG_V_CUTOFF;
				DataTable[REG_QG_V_NEGATIVE] = DIAG_REG_QG_V_NEGATIVE;
				DataTable[REG_QG_C_SET] = DIAG_REG_QG_C_SET;
				DataTable[REG_QG_T_CURRENT] = DIAG_REG_QG_T_CURRENT;
				DataTable[REG_QG_C_POWER] = DIAG_REG_QG_C_POWER;
				DataTable[REG_QG_V_POWER] = DIAG_REG_QG_V_POWER;
				DataTable[REG_QG_C_THRESHOLD] = DIAG_REG_QG_C_THRESHOLD;
				LL_C_Diagnostic(true);
				DELAY_MS(10);
				CONTROL_SetDeviceState(DS_Selftest, SS_QgPulse);
				CONTROL_QG_StartProcess();
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		default:
			return false;
	}

	return true;
}

void DIAG_V_SelfTestFinished()
{
	LL_V_Diagnostic(false);
	if((DataTable[REG_VGS] > DIAG_VGS_THRESHOLD_MIN) && (DataTable[REG_VGS] < DIAG_VGS_THRESHOLD_MAX))
		DataTable[REG_DIAG_RESULT] = DIAG_SUCCESS;
	else
		DataTable[REG_DIAG_RESULT] = DIAG_FAULT;
}

void DIAG_C_SelfTestFinished()
{
	LL_C_Diagnostic(false);
	if((DataTable[REG_QG] > DIAG_QG_THRESHOLD_MIN) && (DataTable[REG_QG] < DIAG_QG_THRESHOLD_MAX))
		DataTable[REG_DIAG_RESULT] = DIAG_SUCCESS;
	else
		DataTable[REG_DIAG_RESULT] = DIAG_FAULT;
}

