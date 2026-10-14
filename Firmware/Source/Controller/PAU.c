// Header
//
#include "PAU.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "LowLevel.h"

// Function prototypes
//
bool PAU_CheckState(PAUState State);


// Functions
//
bool PAU_IsReady()
{
	return PAU_CheckState(PS_Ready);
}
//--------------------------------------

bool PAU_IsConfigReady()
{
	return PAU_CheckState(PS_ConfigReady);
}
//--------------------------------------

bool PAU_InFault()
{
	return PAU_CheckState(PS_Fault);
}
//--------------------------------------

bool PAU_CheckState(PAUState State)
{
	Int16U PAU_State = 0;

	if(DataTable[REG_PAU_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_PAU_CAN_ID], REG_PAU_DEV_STATE, &PAU_State))
		return (PAU_State == State) ? true : false;
	else
		CONTROL_SwitchToFault(DF_PAU_INTERFACE);

	return false;
}
//--------------------------------------

bool PAU_UpdateState(Int16U* Register)
{
	if(DataTable[REG_PAU_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_PAU_CAN_ID], REG_PAU_DEV_STATE, Register))
		return true;

	return false;
}
//--------------------------------------

bool PAU_Configure(Int16U Channel, float Range, Int16U SamplesNumber)
{
	if(DataTable[REG_PAU_EMULATED])
		return true;

	if(BHL_WriteRegister(DataTable[REG_PAU_CAN_ID], REG_PAU_CHANNEL, Channel))
		if(BHL_WriteRegisterFloat(DataTable[REG_PAU_CAN_ID], REG_PAU_RANGE, Range))
			if(BHL_WriteRegisterFloat(DataTable[REG_PAU_CAN_ID], REG_PAU_SAMPLES_NUMBER, SamplesNumber))
				if(BHL_Call(DataTable[REG_PAU_CAN_ID], ACT_PAU_PULSE_CONFIG))
					return true;

	return false;
}
//--------------------------------------

bool PAU_ClearFault()
{
	return (DataTable[REG_PAU_EMULATED]) ? true : BHL_Call(DataTable[REG_PAU_CAN_ID], ACT_PAU_FAULT_CLEAR);
}
//--------------------------------------

bool PAU_ClearWarning()
{
	return (DataTable[REG_PAU_EMULATED]) ? true : BHL_Call(DataTable[REG_PAU_CAN_ID], ACT_PAU_WARNING_CLEAR);
}
//--------------------------------------

bool PAU_ReadMeasuredData(float* Data)
{
	return (DataTable[REG_PAU_EMULATED]) ? true : BHL_ReadRegisterFloat(DataTable[REG_PAU_CAN_ID], REG_PAU_RESULT_CURRENT, Data);
}
//--------------------------------------

void PAU_ShortInput(bool State)
{
	if(DataTable[REG_PAU_EMULATED])
	{
		LL_V_ShortPAU(true);
		return;
	}
	else
		LL_V_ShortPAU(State);
}
//--------------------------------------
