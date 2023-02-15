// Header
//
#include "Regulator.h"
#include "DataTable.h"
#include "LowLevel.h"
#include "ConvertUtils.h"

// Variables
volatile RegulatorParamsStruct RegulatorParams;

// Functions prototypes
//
void REGULATOR_LoggingData(volatile RegulatorParamsStruct* Regulator);
Int16U REGULATOR_DACApplyLimits(Int16S Value, Int16U Offset, Int16U LimitValue);
void REGULATOR_SaveSampleToRingBuffer(volatile RegulatorParamsStruct* Regulator);

// Functions
//
bool REGULATOR_Process(volatile RegulatorParamsStruct* Regulator)
{
	Regulator->Error = (Regulator->Counter == 0) ? 0 : (Regulator->Target - Regulator->SampledData);

	if(Regulator->Error > Regulator->ErrorMax)
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

	float ValueToDAC = (Regulator->DebugMode) ? Regulator->Target : Regulator->Out;

	Regulator->DACSetpoint = REGULATOR_DACApplyLimits(CU_V_VtoDAC(ValueToDAC), Regulator->DACOffset, Regulator->DACLimitValue);

	LL_V_VSetDAC(Regulator->DACSetpoint);

	REGULATOR_LoggingData(Regulator);

	Regulator->Counter++;

	if(!Regulator->Counter)
		return true;
	else
		return false;
}
//-----------------------------------------------

void REGULATOR_SaveSampleToRingBuffer(volatile RegulatorParamsStruct* Regulator)
{
	static Int16U RingCounter = 0;

	RingCounter++;
	RingCounter &= RING_COUNTER_MASK;

	Regulator->RingBuffer[RingCounter] = Regulator->SampledData;
}
//-----------------------------------------------

Int16U REGULATOR_DACApplyLimits(Int16S Value, Int16U Offset, Int16U LimitValue)
{
	Int16S Result = (Int16S)Value + Offset;

	if(Result < 0)
		Result = 0;
	else
	{
		if(Result > LimitValue)
			Result = LimitValue;
	}

	return Result;
}
//-----------------------------------------------

void REGULATOR_LoggingData(volatile RegulatorParamsStruct* Regulator)
{
	static Int16U ScopeLogStep = 0, LocalCounter = 0;

	REGULATOR_SaveSampleToRingBuffer(Regulator);

	// Сброс локального счетчика в начале логгирования
	if(CONTROL_RegulatorValues_Counter == 0)
		LocalCounter = 0;

	if(ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		ScopeLogStep = 0;

		CONTROL_RegulatorOutputValues[LocalCounter] = Regulator->Target;
		CONTROL_RegulatorErrValues[LocalCounter] = (Int16S)(Regulator->Error);
		CONTROL_RegulatorValues_Counter = LocalCounter;

		LocalCounter++;
	}

	// Условие обновления глобального счетчика данных
	if(CONTROL_RegulatorValues_Counter < VALUES_x_SIZE)
		CONTROL_RegulatorValues_Counter = LocalCounter;

	// Сброс локального счетчика
	if(LocalCounter >= VALUES_x_SIZE)
		LocalCounter = 0;
}
//-----------------------------------------------

void REGULATOR_CacheCommonVariables(volatile RegulatorParamsStruct* Regulator)
{
	Regulator->Kp = DataTable[REG_REGULATOR_Kp];
	Regulator->Ki = DataTable[REG_REGULATOR_Ki];
	Regulator->ErrorMax = DataTable[REG_REGULATOR_ERR_MAX];
	Regulator->Qimax = DataTable[REG_REGULATOR_QI_MAX];
	Regulator->FECounterMax = DataTable[REG_FE_COUNTER_MAX];
	Regulator->DACLimitValue = DataTable[REG_DAC_OUTPUT_LIMIT_VALUE];
	Regulator->DACOffset = DataTable[REG_DAC_OFFSET];
	Regulator->DebugMode = DataTable[REG_REGULATOR_DEBUG];
}
//-----------------------------------------------

void REGULATOR_CacheVgsVariables(volatile RegulatorParamsStruct* Regulator)
{
	Regulator->dVg = DataTable[REG_VGS_FAST_RATE];
	Regulator->Counter = DataTable[REG_VGS_V_MAX] / (DataTable[REG_VGS_FAST_RATE] + DataTable[REG_VGS_SLOW_RATE]) / TIMER15_uS;
}
//-----------------------------------------------

void REGULATOR_ResetVariables(volatile RegulatorParamsStruct* Regulator)
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
