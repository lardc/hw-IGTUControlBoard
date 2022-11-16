// Header
//
#include "Regulator.h"
#include "DataTable.h"
#include "LowLevel.h"
#include "ConvertUtils.h"

// Functions prototypes
//
void REGULATOR_LoggingData(volatile RegulatorParamsStruct* Regulator);
Int16U REGULATOR_DACApplyLimits(float Value, Int16U Offset, Int16U LimitValue);

// Functions
//
bool REGULATOR_Process(volatile RegulatorParamsStruct* Regulator)
{
	static float Qi = 0, Qp;

	Regulator->RegulatorError = (Regulator->RegulatorPulseCounter == 0) ? 0 : (Regulator->VFormTable[Regulator->RegulatorPulseCounter] - Regulator->VSen);

	Qp = Regulator->RegulatorError * Regulator->Kp;
	Qi += Regulator->RegulatorError * (Regulator->Ki + Regulator->KiTune);

	float Qi_max = (float)DataTable[REG_REGULATOR_QI_MAX];
	if(Qi > Qi_max)
		Qi = Qi_max;
	else if (Qi < -Qi_max)
		Qi = -Qi_max;

	Regulator->RegulatorOutput = Regulator->VFormTable[Regulator->RegulatorPulseCounter] + Qp + Qi;

	// Выбор источника данных для записи в ЦАП
	float ValueToDAC;
	if(Regulator->DebugMode)
		ValueToDAC = Regulator->VFormTable[Regulator->RegulatorPulseCounter];
	else
		ValueToDAC = Regulator->RegulatorOutput;

	// Проверка границ диапазона ЦАП
	Regulator->DACSetpoint = REGULATOR_DACApplyLimits(CU_V_VToDAC(ValueToDAC), Regulator->DACOffset, Regulator->DACLimitValue);
	LL_V_VSetDAC(Regulator->DACSetpoint);

	if(DataTable[REG_REGULATOR_LOGGING] == 1)
		REGULATOR_LoggingData(Regulator);
	Regulator->RegulatorPulseCounter++;
	if(Regulator->RegulatorPulseCounter >= PULSE_BUFFER_SIZE)
	{
		Regulator->DebugMode = false;
		Regulator->RegulatorPulseCounter = 0;
		Regulator->CTrigRegulatorPulse = 0;
		Qi = 0;
		return true;
	}
	else
		return false;
}
//-----------------------------------------------

Int16U REGULATOR_DACApplyLimits(float Value, Int16U Offset, Int16U LimitValue)
{
	Int16S Result = (Int16S)(Value + Offset);
	if (Result < 0)
		return 0;
	else if (Result > LimitValue)
		return LimitValue;
	else
		return Result;
}
//-----------------------------------------------

void REGULATOR_LoggingData(volatile RegulatorParamsStruct* Regulator)
{
	static Int16U ScopeLogStep = 0, LocalCounter = 0;

	// Сброс локального счетчика в начале логгирования
	if (CONTROL_V_Values_Counter == 0)
		LocalCounter = 0;

	if (ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		ScopeLogStep = 0;

		CONTROL_V_VValues[LocalCounter] = (Int16U)(Regulator->VFormTable[Regulator->RegulatorPulseCounter]);
		CONTROL_V_VSenValues[LocalCounter] = (Int16U)(Regulator->VSen);
		CONTROL_V_CSenValues[LocalCounter] = (Int16U)(Regulator->CSenForm[Regulator->RegulatorPulseCounter]);
		CONTROL_V_RegErrValues[LocalCounter] = (Int16S)(Regulator->RegulatorError);
		CONTROL_V_RegOutValues[LocalCounter] = (Int16S)(Regulator->RegulatorOutput);
		CONTROL_V_VDACRawValues[LocalCounter] = (Int16U)(Regulator->DACSetpoint);

		CONTROL_V_Values_Counter = LocalCounter;

		LocalCounter++;
	}

	// Условие обновления глобального счетчика данных
	if (CONTROL_V_Values_Counter < V_VALUES_x_SIZE)
		CONTROL_V_Values_Counter = LocalCounter;

	// Сброс локального счетчика
	if (LocalCounter >= V_VALUES_x_SIZE)
		LocalCounter = 0;
}
//-----------------------------------------------

void REGULATOR_VGS_FormConfig(volatile RegulatorParamsStruct* Regulator)
{
	Int16U VGSFrontLastPulse = (Int16U)((float)DataTable[REG_VGS_T_V_FRONT] * 1000 / PULSE_PERIOD);
	for (Int16U i = 0; i < PULSE_BUFFER_SIZE; i++)
		Regulator->VFormTable[i] = i < VGSFrontLastPulse ?  (float)((DataTable[REG_VGS_V_MAX] * (i+1)) / VGSFrontLastPulse) : 0;
}
//-----------------------------------------------

void REGULATOR_IGES_FormConfig(volatile RegulatorParamsStruct* Regulator)
{
}
//-----------------------------------------------

void REGULATOR_VGS_FormUpdate (volatile RegulatorParamsStruct* Regulator)
{
	TIM_Stop(TIM15);
	Regulator->ConstantVFirstPulse = Regulator->RegulatorPulseCounter;
	Regulator->ConstantVLastPulse = Regulator->RegulatorPulseCounter + (Int16U)((float)DataTable[REG_VGS_T_V_CONSTANT] * 1000 / PULSE_PERIOD);
	if (Regulator->ConstantVLastPulse > PULSE_BUFFER_SIZE)
		Regulator->ConstantVLastPulse = PULSE_BUFFER_SIZE;
	for (Int16U i = Regulator->RegulatorPulseCounter; i < PULSE_BUFFER_SIZE; i++)
		Regulator->VFormTable[i] = i < Regulator->ConstantVLastPulse ? Regulator->VFormTable[Regulator->RegulatorPulseCounter] : 0;
	TIM_Start(TIM15);
}
//-----------------------------------------------

void REGULATOR_CashVariables(volatile RegulatorParamsStruct* Regulator)
{
	Regulator->Kp = (float)DataTable[REG_REGULATOR_Kp] / 1000;
	Regulator->Ki = (float)DataTable[REG_REGULATOR_Ki] / 1000;
	Regulator->KiTune = (float)DataTable[REG_REGULATOR_TF_Ki] / 1e6;
	Regulator->DebugMode = (bool)DataTable[REG_REGULATOR_DEBUG];
	Regulator->DACOffset = DataTable[REG_DAC_OFFSET];
	Regulator->DACLimitValue = (DAC_MAX_VAL > DataTable[REG_DAC_OUTPUT_LIMIT_VALUE]) ? \
			DataTable[REG_DAC_OUTPUT_LIMIT_VALUE] : DAC_MAX_VAL;
}
//-----------------------------------------------
