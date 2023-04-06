// Header
//
#include "TOCUHP.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "Math.h"

// Variables
//
Int16U	TOCUHP_State;
bool TOCUHP_Emulated = false;
Int16U TOCUHP_NodeID[TOCUHP_NUM_MAX];
Int16U TOCUHP_ActiveUnitsCounter = 0;

// Functions prototypes
//
bool TOCUHP_CheckState(TOCUHPState State);
bool TOCUHP_Call(Int16U Call);
Int16U TOCUHP_CalculateBitMask(float AnodeCurrent);


// Functions
//
void TOCUHP_UpdateCANid()
{
	TOCUHP_ActiveUnitsCounter = 0;

	if(DataTable[REG_TOCUHP0_CAN_ID])
		TOCUHP_NodeID[TOCUHP_ActiveUnitsCounter++] = DataTable[REG_TOCUHP0_CAN_ID];
	if(DataTable[REG_TOCUHP1_CAN_ID])
		TOCUHP_NodeID[TOCUHP_ActiveUnitsCounter++] = DataTable[REG_TOCUHP1_CAN_ID];
	if(DataTable[REG_TOCUHP2_CAN_ID])
		TOCUHP_NodeID[TOCUHP_ActiveUnitsCounter++] = DataTable[REG_TOCUHP2_CAN_ID];
}
//--------------------------------------

void TOCUHP_EmulatedState(bool State)
{
	TOCUHP_Emulated = State;
}
//--------------------------------------

bool TOCUHP_IsEmulatedState()
{
	return (TOCUHP_Emulated | (bool)DataTable[REG_TOCUHP_EMULATED]);
}
//--------------------------------------

bool TOCUHP_IsReady()
{
	return TOCUHP_CheckState(TS_Ready);
}
//--------------------------------------

bool TOCUHP_InFault()
{
	return TOCUHP_CheckState(TS_Fault);
}
//--------------------------------------

bool TOCUHP_ClearFault()
{
	return TOCUHP_Call(ACT_TOCUHP_FAULT_CLEAR);
}
//--------------------------------------

bool TOCUHP_PowerEnable()
{
	return TOCUHP_Call(ACT_TOCUHP_ENABLE_POWER);
}
//--------------------------------------

bool TOCUHP_PowerDisable()
{
	return TOCUHP_Call(ACT_TOCUHP_DISABLE_POWER);
}
//--------------------------------------

bool TOCUHP_ConfigAnodeVoltage(Int16U Voltage)
{
	for(int i = 0; i < TOCUHP_ActiveUnitsCounter; i++)
	{
		if(BHL_WriteRegister(TOCUHP_NodeID[i], DataTable[REG_TOCUHP_VOLTAGE_SETPOINT], Voltage))
			if(BHL_Call(TOCUHP_NodeID[i], ACT_TOCUHP_VOLTAGE_CONFIG))
				continue;

		CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
		return false;
	}

	return true;
}
//--------------------------------------

bool TOCUHP_ConfigAnodeCurrent(float Current)
{
	Int16U TOCUHPx_BitMask[TOCUHP_NUM_MAX];
	Int16U BitMask = 0;

	BitMask = TOCUHP_CalculateBitMask(Current) & TOCUHP_BIT_MASK;
	TOCUHPx_BitMask[0] = BitMask & (Int16U)(powf(2, (Int16U)DataTable[REG_TOCUHP0_BITS]) - 1);
	TOCUHPx_BitMask[1] = (BitMask >> (Int16U)DataTable[REG_TOCUHP0_BITS]) & (Int16U)(powf(2, DataTable[REG_TOCUHP1_BITS]) - 1);
	TOCUHPx_BitMask[2] = (BitMask >> (Int16U)DataTable[REG_TOCUHP1_BITS]) & (Int16U)(powf(2, DataTable[REG_TOCUHP2_BITS]) - 1);

	for(int i = 0; i < TOCUHP_ActiveUnitsCounter; i++)
	{
		if(BHL_WriteRegister(TOCUHP_NodeID[i], DataTable[REG_TOCUHP_GATE_REGISTER], TOCUHPx_BitMask[i]))
			if(BHL_Call(TOCUHP_NodeID[i], ACT_TOCUHP_PULSE_CONFIG))
				continue;

		CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
		return false;
	}

	return true;
}
//--------------------------------------

Int16U TOCUHP_CalculateBitMask(float AnodeCurrent)
{
	float CurrentPerBit = DataTable[REG_QG_V_POWER] / DataTable[REG_TOCUHP_RES_PER_BIT0];
	return (uint16_t)(AnodeCurrent / CurrentPerBit);
}
//-----------------------------------------------

bool TOCUHP_Call(Int16U Call)
{
	for(int i = 0; i < TOCUHP_ActiveUnitsCounter; i++)
	{
		if(!BHL_Call(TOCUHP_NodeID[i], Call))
		{
			CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
			return false;
		}
	}

	return true;
}
//--------------------------------------

bool TOCUHP_CheckState(TOCUHPState State)
{
	Int16U TOCUHP_State;

	if(DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated)
		return true;

	for(int i = 0; i < TOCUHP_ActiveUnitsCounter; i++)
	{
		if(BHL_ReadRegister(TOCUHP_NodeID[i], REG_TOCUHP_DEV_STATE, &TOCUHP_State))
		{
			if(TOCUHP_State != State)
				return false;
		}
		else
		{
			CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
			return false;
		}
	}

	return true;
}
//--------------------------------------
