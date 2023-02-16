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


// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
volatile Int64U CONTROL_TimeCounter = 0;
static Boolean CycleActive = false;
//
volatile float CONTROL_RegulatorOutputValues[VALUES_x_SIZE];
volatile float CONTROL_RegulatorErrValues[VALUES_x_SIZE];
volatile float CONTROL_VoltageValues[VALUES_x_SIZE];
volatile float CONTROL_CurrentValues[VALUES_x_SIZE];
//
volatile Int16U CONTROL_RegulatorValues_Counter = 0;
volatile Int16U CONTROL_Values_Counter = 0;
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

	DataTable[REG_VGS] = 0;
	DataTable[REG_QG] = 0;
	DataTable[REG_QG_T] = 0;
	DataTable[REG_QG_C] = 0;
	DataTable[REG_IGES] = 0;

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
	LL_C_Diagnostic(false);
	LL_C_CStart(false);
	LL_C_CEnable(false);
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

		case ACT_VGS_START:
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

		case ACT_QG_START:
			if(CONTROL_State == DS_Ready)
				CONTROL_SetDeviceState(DS_InProcess, SS_QgPrepare);
			else if(CONTROL_State == DS_InProcess)
				*pUserError = ERR_OPERATION_BLOCKED;
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_IGES_START:
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

		case ACT_STOP_PROCESS:
			if(CONTROL_State == DS_InProcess)
			{
				CONTROL_ForceStopProcess();

				DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
				DataTable[REG_PROBLEM] = PROBLEM_FORCED_STOP;

				CONTROL_SetDeviceState(DS_Ready, SS_None);
			}
			break;

		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
				CONTROL_ResetToDefaultState();
			break;

		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_CAL_VG:
			if(CONTROL_State == DS_Ready)
			{
				CONTROL_ResetOutputRegisters();
				CONTROL_SetDeviceState(DS_InProcess, SS_VcalPrepare);
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

void CONTROL_LogicProcess()
{
	switch(CONTROL_SubState)
	{
		case SS_VcalPrepare:
			CAL_Prepare();

			CONTROL_SetDeviceState(DS_Ready, SS_Vcal);
			CONTROL_V_Start();
			break;

		default:
			break;
	}
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
	MeasureSample SampledData = MEASURE_SampleVgsIges();

	switch(CONTROL_SubState)
	{
		case SS_Vcal:
			CAL_VgsProcess(SampledData);
			break;

		default:
			break;
	}
}
//-----------------------------------------------

void CONTROL_V_Stop()
{
	TIM_Stop(TIM15);
	LL_V_VSetDAC(0);

	CONTROL_ResetHardwareToDefaultState();
}
//------------------------------------------

void CONTROL_V_Start()
{
	LL_SyncOSC(true);

	TIM_Reset(TIM15);
	TIM_Start(TIM15);
}
//-----------------------------------------------

void CONTROL_I_Start()
{

}
//-----------------------------------------------

void CONTROL_I_Stop()
{

}
//-----------------------------------------------

void CONTROL_ForceStopProcess()
{
	CONTROL_I_Stop();
	CONTROL_V_Stop();

	CONTROL_ResetHardwareToDefaultState();
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_SetDeviceState(DS_Fault, SS_None);
	DataTable[REG_FAULT_REASON] = Reason;
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
