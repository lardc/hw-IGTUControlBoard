// Header
#include "Controller.h"

// Includes
#include "SysConfig.h"

// Definitions
typedef struct __SamplingResult
{
	float Ug, UPot, Ig;
} SamplingResult;

// Variables
Int16U REGLTR_MemBuffUg[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffUPot[ADC_SEQ_LENGTH];
Int16U REGLTR_MemBuffIg[ADC_SEQ_LENGTH];
static float Kp, Ki, Qi = 0, PrevSetPoint = 0;
static Int16U Index = 0;

// Forward functions
float REGLTR_GetSetpoint(Int16U i);
SamplingResult REGLTR_GetSample();

// Functions
void REGLTR_Process()
{
	// Получение результата оцифровки и расчёт ошибки
	SamplingResult Sample = REGLTR_GetSample();
	float RegulatorError = PrevSetPoint - Sample.Ug;

	// добавить проверку фолловинг еррор и заглушку для его отработки
	/*
	if(...)
		FollowingErrorCounter = 0;
	else
		FollowingErrorCounter++;
	 */

	float Qp = RegulatorError * Kp;
	Qi += RegulatorError * Ki;

	// Расчёт следующего задания и его корректировка
	float Sepoint = REGLTR_GetSetpoint(Index);
	Sepoint += Qp + Qi;

	// 1. вызов функции пересчёта задания (в мВ) в тики. в ней же проверка на выход за допустимые границы
	// 2. вызов функции записи задания в ЦАП

	// Вызов функции записи в EP отладочной информации
	// 1, 2, 3 оцифрованные значения
	// 4 - задание
	// 5 - скорректированное задание
	// 6 - ошибка

	Index++;
}
//-----------------------------------------

void REGLTR_Init()
{
	Index = Qi = PrevSetPoint = 0;

	// инициализация всех необходимых переменных
	// сброс буферов ДМА
}
//-----------------------------------------

float REGLTR_GetSetpoint(Int16U i)
{
	// функция формирования задания
	return 0;
}
//-----------------------------------------

SamplingResult REGLTR_GetSample()
{
	// получение усреднённого результата оцифровки
	SamplingResult t = {0};
	return t;
}
//-----------------------------------------
