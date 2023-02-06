// Header
//
#include "TOCUHP.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"


// Functions
//
bool TOCUHP_ReadState(Int16U* Register)
{
	if(DataTable[REG_TOCUHP_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_TOCUHP_CAN_ID], REG_TOCUHP_DEV_STATE, Register))
		return true;

	return false;
}
//--------------------------------------

bool TOCUHP_Configure(Int16U Voltage, float AnodeCurrent)
{
	Int16U Mask = TOCUHP_BitMask(AnodeCurrent);

	if(DataTable[REG_TOCUHP_EMULATED])
		return true;

	if(BHL_WriteRegister(DataTable[REG_TOCUHP_CAN_ID], REG_TOCUHP_VOLTAGE_SETPOINT, Voltage))
		if(BHL_WriteRegister(DataTable[REG_TOCUHP_CAN_ID], REG_TOCUHP_GATE_REGISTER, Mask))
			return true;

	return false;
}
//--------------------------------------

bool TOCUHP_CallCommand(uint16_t Command)
{
	if(DataTable[REG_TOCUHP_EMULATED])
		return true;

	return (BHL_Call(DataTable[REG_TOCUHP_CAN_ID], Command));
}
//-----------------------------------------------

bool TOCUHP_AreInStateX(uint16_t State)
{
	if(DataTable[REG_TOCUHP_EMULATED])
		return true;

	Int16U readState;

	if(BHL_ReadRegister(DataTable[REG_TOCUHP_CAN_ID], REG_TOCUHP_DEV_STATE, &readState))
		return (readState == State);

	return false;
}
//-----------------------------------------------

bool TOCUHP_IsInFaultOrDisabled()
{
	if(DataTable[REG_TOCUHP_EMULATED])
			return false;

	Int16U readFault, readDisable;

	if(BHL_ReadRegister(DataTable[REG_TOCUHP_CAN_ID], REG_TOCUHP_FAULT_REASON, &readFault) &&
			BHL_ReadRegister(DataTable[REG_TOCUHP_CAN_ID], TOCUHP_DS_Disabled, &readDisable))
		return ((readDisable == TOCUHP_DS_Disabled) || (readFault == TOCUHP_DS_Fault));

	return false;
}
//-----------------------------------------------

Int16U TOCUHP_BitMask(float AnodeCurrent)
{
	float CurrentPerBit;
	Int16U ActualBitmask = 0;

	// Определение величины тока на бит при заданном напряжении
	CurrentPerBit = (float)TOCUHP_VOLTAGE / DataTable[REG_TOCUHP_RES_PER_BIT];

	// Определение битовой маски для выбранного значения тока
	ActualBitmask = (uint16_t)(AnodeCurrent / CurrentPerBit);
	if(ActualBitmask > TOCUHP_BIT_MASK)
		ActualBitmask = TOCUHP_BIT_MASK;

	return ActualBitmask;
}
//-----------------------------------------------
