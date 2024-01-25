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
#include "TOCUHP.h"
#include "Res.h"
#include "Certification.h"

// Definitions
//
#define I_VCUT_OFF_DAC_DEF			2000

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
volatile Int64U CONTROL_TimeCounter = 0;
volatile Int64U CONTROL_Timeout = 0;
static Boolean CycleActive = false;
Boolean IsImpulse = false;
//
float CONTROL_RegulatorOutputValues[VALUES_x_SIZE];
float CONTROL_RegulatorErrValues[VALUES_x_SIZE];
float CONTROL_VoltageValues[VALUES_x_SIZE];
float CONTROL_CurrentValues[VALUES_x_SIZE];
//
Int16U CONTROL_RegulatorValues_Counter = 0;
Int16U CONTROL_Values_Counter = 0;
//


/// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
void CONTROL_LogicProcess();

// Functions
//
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[FEP_COUNT] = {EP_VOLTAGE, EP_CURRENT, EP_REGULATOR_ERR, EP_REGULATOR_OUTPUT};
	Int16U EPSized[FEP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE, VALUES_x_SIZE, VALUES_x_SIZE};
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
	TOCUHP_UpdateCANid();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	CONTROL_ResetHardwareToDefaultState();
	IsImpulse = false;

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
	LL_QgProtection(false);

	LL_V_VSetDAC(0);
	LL_I_SetDAC(0);
	LL_ExDACVCutoff(I_VCUT_OFF_DAC_DEF);
	LL_ExDACVNegative(0);

	IsImpulse = false;

	ADC_Disable(ADC1);
	ADC_Disable(ADC3);

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
			{
				if(TOCUHP_PowerEnable())
				{
					CONTROL_Timeout = CONTROL_TimeCounter + POWER_ON_TIMEOUT;
					CONTROL_SetDeviceState(DS_Ready, SS_PowerOnProcess);
				}
			}
			else if(CONTROL_State != DS_Ready)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if((CONTROL_State == DS_Ready) || (CONTROL_State == DS_InProcess))
			{
				if(TOCUHP_PowerDisable())
					CONTROL_ResetToDefaultState();
			}
			else if(CONTROL_State != DS_None)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_START_VGS:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();

				if(DataTable[REG_SERTIFICATION])
				{
					SertSet_V = (pFloat32)(&DataTable[REG_VGS_V_MAX]);
					SertSet_I = (pFloat32)(&DataTable[REG_VGS_I_TRIG]);
					SertResult_V = (pFloat32)(&DataTable[REG_VGS_RESULT]);
					SertResult_I = (pFloat32)(&DataTable[REG_VGS_I_RESULT]);
					Mode = FeedBack;
					CONTROL_SetDeviceState(DS_InProcess, SS_Sert_V_Prepare);
				}
				else
				{
					if(DataTable[REG_VGS_V_MAX] < VGS_VOLTAGE_MAX)
						CONTROL_SetDeviceState(DS_InProcess, SS_VgsPrepare);
					else
						*pUserError = ERR_WRONG_PARAMS;
				}
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_QG:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_QgPrepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_IGES:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();

				if(DataTable[REG_SERTIFICATION])
				{
					float Current = V_I_R2_MAX;

					SertSet_V = (pFloat32)(&DataTable[REG_IGES_V]);
					SertSet_I = &Current;
					SertResult_V = NULL;
					SertResult_I = NULL;
					Mode = Parametric;

					CONTROL_SetDeviceState(DS_InProcess, SS_Sert_V_Prepare);
				}
				else
					CONTROL_SetDeviceState(DS_InProcess, SS_IgesPrepare);
			}
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_START_RES:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_ResPrepare);
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
				CONTROL_SetDeviceState(DS_SelfTest, SS_V_Prepare_Voltage);
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
			if(!PAU_ClearFault())
				CONTROL_SwitchToFault(DF_PAU_INTERFACE);
			else if(TOCUHP_ClearFault())
					{
						if(CONTROL_State == DS_Fault)
						{
							LL_Indication(false);
							CONTROL_ResetToDefaultState();
						}
					}
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
//-----------------------------------------------

void CONTROL_LogicProcess()
{
	if(CONTROL_State == DS_InProcess)
	{
		if(!CONTROL_IsSafetyOk())
			return;

		switch(CONTROL_SubState)
		{
			case SS_PowerOnProcess:
				if(TOCUHP_IsReady())
				{
					if(PAU_IsReady())
						CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
				else
				{
					if(CONTROL_TimeCounter > CONTROL_Timeout)
						CONTROL_SwitchToFault(DF_TOCUHP_PWR_ON_TIMEOUT);
				}
				break;

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

			case SS_ResPrepare:
				RES_Prepare();
				break;

			case SS_Sert_V_Prepare:
				SERT_V_Prepare();
				break;

			case SS_Sert_I_Prepare:
				SERT_I_Prepare();
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

		case SS_ResProcess:
			RES_Process();
			break;

		case SS_Sert_V_Process:
			SERT_V_Process();
			break;

		case SS_Sert_I_Process:
			SERT_I_Process();
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
	if(Reason == DF_PAU_INTERFACE || Reason == DF_TOCUHP_INTERFACE)
	{
		BHLError Error = BHL_GetError();
		DataTable[REG_EXT_UNIT_ERROR_CODE] = Error.ErrorCode;
		DataTable[REG_EXT_UNIT_FUNCTION] = Error.Func;
		DataTable[REG_EXT_UNIT_EXT_DATA] = Error.ExtData;
	}

	CONTROL_SetDeviceState(DS_Fault, SS_None);

	if(DataTable[REG_FAULT_REASON] == DF_NONE)
		DataTable[REG_FAULT_REASON] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;

	QG_ResetConfigStageToDefault();
	CONTROL_ResetHardwareToDefaultState();
}
//------------------------------------------

void CONTROL_HandleExternalLamp(bool IsImpulse)
{
	static Int64U ExternalLampCounter = 0;

	if(CONTROL_State != DS_None)
	{
		if(CONTROL_State == DS_Fault)
		{
			if(++ExternalLampCounter > TIME_FAULT_LED_BLINK)
			{
				LL_ToggleIndication();
				ExternalLampCounter = 0;
			}
		}
		else
			{
				if(IsImpulse)
				{
					LL_Indication(true);
					ExternalLampCounter = CONTROL_TimeCounter + EXT_LAMP_ON_STATE_TIME;
				}
				else
				{
					if(CONTROL_TimeCounter >= ExternalLampCounter)
						LL_Indication(false);
				}
			}
	}
}
//-----------------------------------------------

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
