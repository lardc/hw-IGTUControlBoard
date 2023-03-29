// Header
#include "Controller.h"
//
// Includes
#include "DeviceProfile.h"
#include "LowLevel.h"
#include "Diagnostic.h"
#include "BCCIxParams.h"
#include "Delay.h"
#include "Calibration.h"
#include "Vgs.h"
#include "Iges.h"
#include "Qg.h"
#include "SelfTest.h"
#include "BCCIMHighLevel.h"
#include "PAU.h"
#include "Constraints.h"
#include "ConvertUtils.h"

// Definitions
//
#define I_VCUT_OFF_DAC_DEF			2000

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
volatile Int64U CONTROL_TimeCounter = 0;
static Boolean CycleActive = false;
//
float CONTROL_RegulatorOutputValues[VALUES_x_SIZE1];
float CONTROL_RegulatorErrValues[VALUES_x_SIZE1];
float CONTROL_VoltageValues[VALUES_x_SIZE2];
float CONTROL_CurrentValues[VALUES_x_SIZE2];
//
Int16U CONTROL_RegulatorValues_Counter = 0;
Int16U CONTROL_Values_Counter = 0;
//


/// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
void CONTROL_ResetHardwareToDefaultState();
void CONTROL_LogicProcess();
void CONTROL_ResetOutputRegisters();

// Functions
//
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[FEP_COUNT] = {EP_VOLTAGE, EP_CURRENT, EP_REGULATOR_ERR, EP_REGULATOR_OUTPUT};
	Int16U EPSized[FEP_COUNT] = {VALUES_x_SIZE2, VALUES_x_SIZE2, VALUES_x_SIZE1, VALUES_x_SIZE1};
	pInt16U EPCounters[FEP_COUNT] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_RegulatorValues_Counter, (pInt16U)&CONTROL_RegulatorValues_Counter};

	pFloat32 EPDatas[FEP_COUNT] = {(pFloat32)CONTROL_VoltageValues, (pFloat32)CONTROL_CurrentValues,
			(pFloat32)CONTROL_RegulatorErrValues, (pFloat32)CONTROL_RegulatorOutputValues};

	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, CAN_NID);

	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitFEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Сброс значений
	DEVPROFILE_ResetControlSection();

	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	CONTROL_ResetHardwareToDefaultState();

	CONTROL_SetDeviceState(DS_None, SS_None);
}
//------------------------------------------

void CONTROL_ResetOutputRegisters()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;

	DataTable[REG_VGS_RESULT] = 0;
	DataTable[REG_VGS_I_RESULT] = 0;
	DataTable[REG_QG_RESULT] = 0;
	DataTable[REG_QG_I_DURATION_RESULT] = 0;
	DataTable[REG_QG_I_RESULT] = 0;
	DataTable[REG_IGES_RESULT] = 0;

	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();
}
//------------------------------------------

void CONTROL_ResetHardwareToDefaultState()
{
	LL_SyncTOCUHP(false);
	LL_SyncOSC(false);
	LL_SyncPAU(false);
	LL_V_ShortOut(true);
	LL_V_ShortPAU(true);
	LL_V_Diagnostic(false);
	LL_I_Diagnostic(false);
	LL_I_Start(false);
	LL_I_Enable(false);

	LL_V_VSetDAC(0);
	LL_I_SetDAC(0);
	LL_ExDACVCutoff(I_VCUT_OFF_DAC_DEF);
	LL_ExDACVNegative(0);

	CONTROL_SwitchOutMUX(Voltage);
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_LogicProcess();

	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;

	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
				CONTROL_SetDeviceState(DS_Ready, SS_None);
			else if(CONTROL_State != DS_Ready)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if((CONTROL_State == DS_Ready) || (CONTROL_State == DS_InProcess))
				CONTROL_ResetToDefaultState();
			else if(CONTROL_State != DS_None)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_START_VGS:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_VgsPrepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_QG:
			if(CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_InProcess, SS_QgPrepare);
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_IGES:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_IgesPrepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_SELF_TEST:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_NONE;
				CONTROL_SetDeviceState(DS_SelfTest, SS_VoltagePrepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_STOP_PROCESS:
			if(CONTROL_State == DS_InProcess)
			{
				CONTROL_StopHighPriorityProcesses();

				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				DataTable[REG_PROBLEM] = PROBLEM_FORCED_STOP;

				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			break;

		case ACT_CLR_FAULT:
			if(PAU_ClearFault())
			{
				if(CONTROL_State == DS_Fault)
					CONTROL_ResetToDefaultState();
			}
			else
				CONTROL_SwitchToFault(DF_PAU_INTERFACE);
			break;

		case ACT_CLR_WARNING:
			if(PAU_ClearWarning())
				DataTable[REG_WARNING] = WARNING_NONE;
			else
			{
				DataTable[REG_WARNING] = WARNING_NONE;
				CONTROL_SwitchToFault(DF_PAU_INTERFACE);
			}
			break;

		case ACT_CAL_V:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_Cal_V_Prepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_CAL_I:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_Cal_I_Prepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		default:
			if(CONTROL_State == DS_None)
				return DIAG_HandleDiagnosticAction(ActionID, pUserError);
			else
				*pUserError = ERR_OPERATION_BLOCKED;
	}
	return true;
}
//-----------------------------------------------

bool CONTROL_IsSafetyOk()
{
	if(CONTROL_State == DS_InProcess && LL_SafetyState() && !DataTable[REG_MUTE_SAFETY])
	{
		CONTROL_StopHighPriorityProcesses();

		DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
		DataTable[REG_PROBLEM] = PROBLEM_SAFETY_VIOLATION;

		CONTROL_SetDeviceState(DS_Ready, SS_None);

		return false;
	}

	return true;
}

void CONTROL_LogicProcess()
{
	if(CONTROL_State == DS_InProcess)
	{
		if(!CONTROL_IsSafetyOk())
			return;

		switch(CONTROL_SubState)
		{
			case SS_Cal_V_Prepare:
				CAL_V_Prepare();
				break;

			case SS_Cal_I_Prepare:
				CAL_I_Prepare();
				break;

			case SS_VgsPrepare:
				VGS_Prepare();
				break;

			case SS_IgesPrepare:
				IGES_Prepare();
				break;

			case SS_IgesSaveResult:
				IGES_SaveResults();
				break;

			case SS_QgPrepare:
				QG_Prepare();
				break;

			case SS_QgSaveResult:
				QG_SaveResult();
				break;

			default:
				break;
		}
	}

	if(CONTROL_State == DS_SelfTest)
		ST_Process();
}
//-----------------------------------------------

void CONTROL_SwitchOutMUX(CommutationState Commutation)
{
	static CommutationState LastCommutation = Current;

	if(LastCommutation != Commutation)
	{
		if(Commutation == Current)
			LL_OutMultiplexCurrent();
		else
			LL_OutMultiplexVoltage();

		LastCommutation = Commutation;

		DELAY_MS(20);
	}
}
//-----------------------------------------------

void CONTROL_HighPriorityProcess()
{
	switch(CONTROL_SubState)
	{
		case SS_Cal_V_Process:
			CAL_V_CalProcess();
			break;

		case SS_Cal_I_Process:
			CAL_I_CalProcess();
			break;

		case SS_VgsProcess:
			VGS_Process();
			break;

		case SS_IgesProcess:
			IGES_Process();
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_StopHighPriorityProcesses()
{
	TIM_Stop(TIM15);
	CONTROL_ResetHardwareToDefaultState();
}
//------------------------------------------

void CONTROL_StartHighPriorityProcesses()
{
	TIM_Reset(TIM15);
	TIM_Start(TIM15);
}
//-----------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	if(Reason == DF_PAU_INTERFACE)
	{
		BHLError Error = BHL_GetError();
		DataTable[REG_PAU_ERROR_CODE] = Error.ErrorCode;
		DataTable[REG_PAU_FUNCTION] = Error.Func;
		DataTable[REG_PAU_EXT_DATA] = Error.ExtData;
	}

	CONTROL_SetDeviceState(DS_Fault, SS_None);
	DataTable[REG_FAULT_REASON] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState, DeviceSubState NewSubState)
{
	CONTROL_State = NewState;
	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_STATE] = NewState;
	DataTable[REG_SUB_STATE] = NewSubState;
}
//------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------
