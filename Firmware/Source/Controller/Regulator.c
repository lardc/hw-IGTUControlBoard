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

	Regulator->RegulatorError = (Regulator->RegulatorPulseCounter == 0) ? 0 : (Regulator->UFormTable[Regulator->RegulatorPulseCounter] - Regulator->UMeasured);

	Qp = Regulator->RegulatorError * Regulator->Kp;
	Qi += Regulator->RegulatorError * (Regulator->Ki + Regulator->KiTune);

	float Qi_max = (float)DataTable[REG_REGULATOR_QI_MAX];
	if(Qi > Qi_max)
		Qi = Qi_max;
	else if (Qi < -Qi_max)
		Qi = -Qi_max;

	Regulator->RegulatorOutput = Regulator->UFormTable[Regulator->RegulatorPulseCounter] + Qp + Qi;

	// Выбор источника данных для записи в ЦАП
	float ValueToDAC;
	if(Regulator->DebugMode)
		ValueToDAC = Regulator->UFormTable[Regulator->RegulatorPulseCounter];
	else
		ValueToDAC = Regulator->RegulatorOutput;

	// Проверка границ диапазона ЦАП
	Regulator->DACSetpoint = REGULATOR_DACApplyLimits(CU_UUToDAC(ValueToDAC), Regulator->DACOffset, Regulator->DACLimitValue);
	LL_UUSetDAC(Regulator->DACSetpoint);

	if(DataTable[REG_REGULATOR_LOGGING] == 1)
		REGULATOR_LoggingData(Regulator);
	Regulator->RegulatorPulseCounter++;
	if(Regulator->RegulatorPulseCounter >= PULSE_BUFFER_SIZE)
	{
		Regulator->DebugMode = false;
		Regulator->RegulatorPulseCounter = 0;
		Regulator->ITrigRegulatorPulse = 0;
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
	if (CONTROL_Values_Counter == 0)
		LocalCounter = 0;

	if (ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		ScopeLogStep = 0;

		CONTROL_UUValues[LocalCounter] = (Int16U)(Regulator->UFormTable[Regulator->RegulatorPulseCounter]);
		CONTROL_UUMeasValues[LocalCounter] = (Int16U)(Regulator->UMeasured);
		CONTROL_UIMeasValues[LocalCounter] = (Int16U)(Regulator->IFormMeasured[Regulator->RegulatorPulseCounter]);
		CONTROL_RegulatorErr[LocalCounter] = (Int16S)(Regulator->RegulatorError);
		CONTROL_RegulatorOutput[LocalCounter] = (Int16S)(Regulator->RegulatorOutput);
		CONTROL_DACRawData[LocalCounter] = (Int16U)(Regulator->DACSetpoint);

		CONTROL_Values_Counter = LocalCounter;

		LocalCounter++;
	}

	// Условие обновления глобального счетчика данных
	if (CONTROL_Values_Counter < U_VALUES_x_SIZE)
		CONTROL_Values_Counter = LocalCounter;

	// Сброс локального счетчика
	if (LocalCounter >= U_VALUES_x_SIZE)
		LocalCounter = 0;
}
//-----------------------------------------------

void REGULATOR_UFormConfig(volatile RegulatorParamsStruct* Regulator)
{
	Int16U UFrontLastPulse = (Int16U)((float)DataTable[REG_U_T_UFRONT] * 1000 / PULSE_PERIOD);
	for (Int16U i = 0; i < PULSE_BUFFER_SIZE; i++)
		Regulator->UFormTable[i] = i < UFrontLastPulse ?  (float)((DataTable[REG_U_UMAX] * (i+1)) / UFrontLastPulse) : 0;
}
//-----------------------------------------------

void REGULATOR_UFormUpdate (volatile RegulatorParamsStruct* Regulator)
{
	TIM_Stop(TIM15);
	Regulator->ConstantUFirstPulse = Regulator->RegulatorPulseCounter;
	Regulator->ConstantULastPulse = Regulator->RegulatorPulseCounter + (Int16U)((float)DataTable[REG_U_T_UCONSTANT] * 1000 / PULSE_PERIOD);
	if (Regulator->ConstantULastPulse > PULSE_BUFFER_SIZE)
		Regulator->ConstantULastPulse = PULSE_BUFFER_SIZE;
	for (Int16U i = Regulator->RegulatorPulseCounter; i < PULSE_BUFFER_SIZE; i++)
		Regulator->UFormTable[i] = i < Regulator->ConstantULastPulse ? Regulator->UFormTable[Regulator->RegulatorPulseCounter] : 0;
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
