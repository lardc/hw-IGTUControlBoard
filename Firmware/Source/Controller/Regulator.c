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

	Regulator->RegulatorError =
			(Regulator->RegulatorStepCounter == 0) ?
					0 : (Regulator->VFormTable[Regulator->RegulatorStepCounter] - Regulator->VSen);

	Qp = Regulator->RegulatorError * Regulator->Kp;
	Qi += Regulator->RegulatorError * Regulator->Ki;

	float Qi_max = (float)DataTable[REG_REGULATOR_QI_MAX];
	if(Qi > Qi_max)
		Qi = Qi_max;
	else if(Qi < -Qi_max)
		Qi = -Qi_max;

	Regulator->RegulatorOutput = Regulator->VFormTable[Regulator->RegulatorStepCounter] + Qp + Qi;

	// Выбор источника данных для записи в ЦАП
	float ValueToDAC;
	if(Regulator->DebugMode)
		ValueToDAC = Regulator->VFormTable[Regulator->RegulatorStepCounter];
	else
		ValueToDAC = Regulator->RegulatorOutput;

	// Проверка границ диапазона ЦАП
	Regulator->DACSetpoint = REGULATOR_DACApplyLimits(CU_V_VToDAC(ValueToDAC), Regulator->DACOffset,
			Regulator->DACLimitValue);
	LL_V_VSetDAC(Regulator->DACSetpoint);

	if(DataTable[REG_REGULATOR_LOGGING] == 1)
		REGULATOR_LoggingData(Regulator);
	Regulator->RegulatorStepCounter++;
	if(Regulator->RegulatorStepCounter >= STEP_BUFFER_SIZE)
	{
		Regulator->DebugMode = false;
		Regulator->RegulatorStepCounter = 0;
		Regulator->CTrigRegulatorStep = 0;
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
	if(Result < 0)
		return 0;
	else if(Result > LimitValue)
		return LimitValue;
	else
		return Result;
}
//-----------------------------------------------

void REGULATOR_LoggingData(volatile RegulatorParamsStruct* Regulator)
{
	static Int16U ScopeLogStep = 0, LocalCounter = 0;

	// Сброс локального счетчика в начале логгирования
	if(CONTROL_V_Values_Counter == 0)
		LocalCounter = 0;

	if(ScopeLogStep++ >= DataTable[REG_SCOPE_STEP])
	{
		ScopeLogStep = 0;

		CONTROL_V_VValues[LocalCounter] = (Int16U)(Regulator->VFormTable[Regulator->RegulatorStepCounter]);
		CONTROL_V_VSenValues[LocalCounter] = (Int16U)(Regulator->VSen);
		CONTROL_V_CSenValues[LocalCounter] = (Int16U)(Regulator->CSen);
		CONTROL_V_RegErrValues[LocalCounter] = (Int16S)(Regulator->RegulatorError);
		CONTROL_V_Values_Counter = LocalCounter;

		LocalCounter++;
	}

	// Условие обновления глобального счетчика данных
	if(CONTROL_V_Values_Counter < V_VALUES_x_SIZE)
		CONTROL_V_Values_Counter = LocalCounter;

	// Сброс локального счетчика
	if(LocalCounter >= V_VALUES_x_SIZE)
		LocalCounter = 0;
}
//-----------------------------------------------

void REGULATOR_VGS_FormConfig(volatile RegulatorParamsStruct* Regulator)
{
	Int16U VGSFrontLastStep = (Int16U)((float)DataTable[REG_VGS_T_V_FRONT] * 1000 / STEP_PERIOD);
	for(Int16U i = 0; i < STEP_BUFFER_SIZE; i++)
		Regulator->VFormTable[i] =
				i < VGSFrontLastStep ? (Int16U)((1000 * DataTable[REG_VGS_V_MAX] * (i + 1)) / VGSFrontLastStep) : 0;
}
//-----------------------------------------------

void REGULATOR_IGES_FormConfig(volatile RegulatorParamsStruct* Regulator)
{
	Int16U IGESFrontLastStep = (Int16U)((float)(DataTable[REG_IGES_T_V_FRONT]) * 1000 / STEP_PERIOD);
	Regulator->ConstantVFirstStep = IGESFrontLastStep + 1;
	Int16U IGESLastStep = (Int16U)((float)(DataTable[REG_IGES_T_V_FRONT] + DataTable[REG_IGES_T_V_CONSTANT]) * 1000
			/ STEP_PERIOD);
	for(Int16U i = 0; i < STEP_BUFFER_SIZE; i++)
	{
		if(i < IGESFrontLastStep)
			Regulator->VFormTable[i] = (Int16U)((1000 * DataTable[REG_IGES_V] * (i + 1)) / IGESFrontLastStep);
		else if(i < IGESLastStep)
			Regulator->VFormTable[i] = (Int16U)(1000 * DataTable[REG_IGES_V]);
		else
			Regulator->VFormTable[i] = 0;
	}
}
//-----------------------------------------------

void REGULATOR_VGS_FormUpdate(volatile RegulatorParamsStruct* Regulator)
{
	TIM_Stop(TIM15);
	Regulator->ConstantVFirstStep = Regulator->RegulatorStepCounter;
	Regulator->ConstantVLastStep = Regulator->RegulatorStepCounter
			+ (Int16U)((Int16U) DataTable[REG_VGS_T_V_CONSTANT] * 1000 / STEP_PERIOD);
	if(Regulator->ConstantVLastStep > STEP_BUFFER_SIZE)
		Regulator->ConstantVLastStep = STEP_BUFFER_SIZE;
	for(Int16U i = Regulator->RegulatorStepCounter; i < STEP_BUFFER_SIZE; i++)
		Regulator->VFormTable[i] =
				i < Regulator->ConstantVLastStep ? Regulator->VFormTable[Regulator->RegulatorStepCounter] : 0;
	TIM_Start(TIM15);
}
//-----------------------------------------------

bool REGULATOR_IGES_CheckVConstant(volatile RegulatorParamsStruct* Regulator)
{
	return Regulator->ConstantVFirstStep == Regulator->RegulatorStepCounter;
}
//-----------------------------------------------

void REGULATOR_CashVariables(volatile RegulatorParamsStruct* Regulator)
{
	Regulator->Kp = (float)DataTable[REG_REGULATOR_Kp];
	Regulator->Ki = (float)DataTable[REG_REGULATOR_Ki];
	Regulator->DebugMode = (bool)DataTable[REG_REGULATOR_DEBUG];
	Regulator->DACOffset = DataTable[REG_DAC_OFFSET];
	Regulator->DACLimitValue =
			(DAC_MAX_VAL > DataTable[REG_DAC_OUTPUT_LIMIT_VALUE]) ? DataTable[REG_DAC_OUTPUT_LIMIT_VALUE] : DAC_MAX_VAL;
}
//-----------------------------------------------
