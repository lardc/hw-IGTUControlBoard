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
volatile float* TOCUHP_NodeID[TOCUHP_NUM_MAX];

// Functions prototypes
//
bool TOCUHP_CheckState(TOCUHPState State);
bool TOCUHP_OpResult();
bool TOCUHP_Call(Int16U Call);
Int16U TOCUHP_CalculateBitMask(float AnodeCurrent);
bool TOCUHP_CompareRegister(Int16U Register, Int16U Vlaue);

// Functions
//
void TOCUHP_UpdateCANid()
{
	TOCUHP_NodeID[0] = &DataTable[REG_TOCUHP0_CAN_ID];
	TOCUHP_NodeID[1] = &DataTable[REG_TOCUHP1_CAN_ID];
	TOCUHP_NodeID[2] = &DataTable[REG_TOCUHP2_CAN_ID];
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

bool TOCUHP_CheckOpResult()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? true : TOCUHP_OpResult();
}
//--------------------------------------

bool TOCUHP_IsReady()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? true : TOCUHP_CheckState(TS_Ready);
}
//--------------------------------------

bool TOCUHP_InFault()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? false : TOCUHP_CheckState(TS_Fault);
}
//--------------------------------------

bool TOCUHP_ClearFault()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? true : TOCUHP_Call(ACT_TOCUHP_FAULT_CLEAR);
}
//--------------------------------------

bool TOCUHP_PowerEnable()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? true : TOCUHP_Call(ACT_TOCUHP_ENABLE_POWER);
}
//--------------------------------------

bool TOCUHP_PowerDisable()
{
	return (DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated) ? true : TOCUHP_Call(ACT_TOCUHP_DISABLE_POWER);
}
//--------------------------------------

bool TOCUHP_ConfigAnodeVoltage(Int16U Voltage)
{
	if((DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated))
		return true;

	for(int i = 0; i < TOCUHP_NUM_MAX; i++)
	{
		if(*TOCUHP_NodeID[i])
		{
			if(BHL_WriteRegister(*TOCUHP_NodeID[i], REG_TOCUHP_VOLTAGE_SETPOINT, Voltage))
				if(BHL_Call(*TOCUHP_NodeID[i], ACT_TOCUHP_VOLTAGE_CONFIG))
					continue;

			CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
			return false;
		}
	}

	return true;
}
//--------------------------------------

bool TOCUHP_ConfigAnodeCurrent(float Current)
{
	Int16U TOCUHPx_BitMask[TOCUHP_NUM_MAX];
	Int16U BitMask = 0;

	if((DataTable[REG_TOCUHP_EMULATED] || TOCUHP_Emulated))
		return true;

	BitMask = TOCUHP_CalculateBitMask(Current) & TOCUHP_BIT_MASK;
	TOCUHPx_BitMask[0] = BitMask & (Int16U)(powf(2, (Int16U)DataTable[REG_TOCUHP0_BITS]) - 1);
	TOCUHPx_BitMask[1] = (BitMask >> (Int16U)DataTable[REG_TOCUHP0_BITS]) & (Int16U)(powf(2, DataTable[REG_TOCUHP1_BITS]) - 1);
	TOCUHPx_BitMask[2] = (BitMask >> (Int16U)DataTable[REG_TOCUHP1_BITS]) & (Int16U)(powf(2, DataTable[REG_TOCUHP2_BITS]) - 1);

	for(int i = 0; i < TOCUHP_NUM_MAX; i++)
	{
		if(*TOCUHP_NodeID[i])
		{
			if(BHL_WriteRegister(*TOCUHP_NodeID[i], REG_TOCUHP_GATE_REGISTER, TOCUHPx_BitMask[i]))
				if(BHL_Call(*TOCUHP_NodeID[i], ACT_TOCUHP_PULSE_CONFIG))
					continue;

			CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
			return false;
		}
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
	for(int i = 0; i < TOCUHP_NUM_MAX; i++)
	{
		if(*TOCUHP_NodeID[i])
		{
			if(!BHL_Call(*TOCUHP_NodeID[i], Call))
			{
				CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
				return false;
			}
		}
	}

	return true;
}
//--------------------------------------

bool TOCUHP_CheckState(TOCUHPState State)
{
	static Int64U CheckStatePeriodCounter = 0;

	if(CONTROL_TimeCounter >= CheckStatePeriodCounter)
	{
		CheckStatePeriodCounter = CONTROL_TimeCounter + BCCIM_TIMEOUT_TICKS;

		return TOCUHP_CompareRegister(REG_TOCUHP_DEV_STATE, State);
	}

	return false;
}
//--------------------------------------

bool TOCUHP_OpResult()
{
	return TOCUHP_CompareRegister(REG_TOCUHP_OP_RESULT, OPRESULT_OK);
}
//--------------------------------------

bool TOCUHP_CompareRegister(Int16U Register, Int16U Vlaue)
{
	Int16U ReadValue = 0;

	for(int i = 0; i < TOCUHP_NUM_MAX; i++)
	{
		if(*TOCUHP_NodeID[i])
		{
			if(BHL_ReadRegister(*TOCUHP_NodeID[i], Register, &ReadValue))
			{
				if(ReadValue != Vlaue)
					return false;
			}
			else
			{
				CONTROL_SwitchToFault(DF_TOCUHP_INTERFACE);
				return false;
			}
		}
	}

	return true;
}
//--------------------------------------
