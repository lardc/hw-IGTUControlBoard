// Header
//
#include "Regulator.h"

// Includes
//
#include "DataTable.h"
#include "LowLevel.h"
#include "ConvertUtils.h"
#include "Logging.h"

// Variables
//
RegulatorParamsStruct RegulatorParams;
LogParamsStruct RegulatorLog;


// Functions prototypes
//
Int16U REGULATOR_DACApplyLimits(Int16S Value, Int16U LimitValue);

// Functions
//
bool REGULATOR_Process(RegulatorParamsStruct* Regulator)
{
	Regulator->Error = (Regulator->Counter == 0) ? 0 : (Regulator->Target - Regulator->SampledData);

	if(Regulator->ParametricMode == FeedBack && Regulator->Error > Regulator->ErrorMax)
	{
		Regulator->FECounter++;

		if(Regulator->FECounter > Regulator->FECounterMax)
		{
			Regulator->FollowingError = true;
			return true;
		}
	}
	else
		Regulator->FECounter = 0;

	Regulator->Qp = Regulator->Error * Regulator->Kp;
	Regulator->Qi += Regulator->Error * Regulator->Ki;

	if(Regulator->Qi > Regulator->Qimax)
		Regulator->Qi = Regulator->Qimax;
	else if(Regulator->Qi < -Regulator->Qimax)
		Regulator->Qi = -Regulator->Qimax;

	Regulator->Out = Regulator->Target + Regulator->Qp + Regulator->Qi;

	float ValueToDAC = (Regulator->ParametricMode == Parametric) ? Regulator->Target : Regulator->Out;
	Regulator->DACSetpoint = REGULATOR_DACApplyLimits(CU_V_VtoDAC(ValueToDAC), Regulator->DACLimitValue);

	LL_V_VSetDAC(Regulator->DACSetpoint);

	LOG_LoggingData(&RegulatorLog);

	Regulator->Counter--;

	if(Regulator->Counter)
		return false;
	else
		return true;
}
//-----------------------------------------------

Int16U REGULATOR_DACApplyLimits(Int16S Value, Int16U LimitValue)
{
	if(Value < 0)
		Value = 0;
	else
	{
		if(Value > LimitValue)
			Value = LimitValue;
	}

	return Value;
}
//-----------------------------------------------

void REGULATOR_CacheVariables(RegulatorParamsStruct* Regulator)
{
	Regulator->Kp = DataTable[REG_REGULATOR_Kp];
	Regulator->Ki = DataTable[REG_REGULATOR_Ki];
	Regulator->ErrorMax = DataTable[REG_REGULATOR_ERR_MAX];
	Regulator->Qimax = DataTable[REG_REGULATOR_QI_MAX];
	Regulator->FECounterMax = DataTable[REG_REGULATOR_FE_COUNTER];
	Regulator->DACLimitValue = DataTable[REG_DAC_OUTPUT_LIMIT_VALUE];
	Regulator->DACLimitValue = DataTable[REG_DAC_OUTPUT_LIMIT_VALUE];

	RegulatorLog.DataA = &Regulator->Out;
	RegulatorLog.DataB = &Regulator->Error;
	RegulatorLog.LogBufferA = &CONTROL_RegulatorOutputValues[0];
	RegulatorLog.LogBufferB = &CONTROL_RegulatorErrValues[0];
	RegulatorLog.LogBufferCounter = &CONTROL_RegulatorValues_Counter;
}
//-----------------------------------------------

void REGULATOR_ResetVariables(RegulatorParamsStruct* Regulator)
{
	Regulator->Target = 0;
	Regulator->SampledData = 0;
	Regulator->DACSetpoint = 0;
	Regulator->Error = 0;
	Regulator->Qi = 0;
	Regulator->Qp = 0;
	Regulator->Out = 0;
	Regulator->FECounter = 0;
	Regulator->FollowingError = false;
}
//-----------------------------------------------

void REGULATOR_Mode(RegulatorParamsStruct* Regulator, RegulatorMode Mode)
{
	Regulator->ParametricMode = (DataTable[REG_REGULATOR_PARAMETRIC] == Parametric) ? Parametric : Mode;
}
//-----------------------------------------------
