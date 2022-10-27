#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Включение блока
#define ACT_DISABLE_POWER				2	// Выключение блока
#define ACT_CLR_FAULT					3	// Очистка всех fault
#define ACT_CLR_WARNING					4	// Очистка всех warning
// Источник напряжения
#define ACT_DBG_U_U_SET					50	// Установить значение напряжения затвора из регистра DBG [mV]
#define ACT_DBG_U_SHORT					51	// Закоротить выход источника напряжения (регистр DBG = 1 -> закоротка, DBG = 0 -> разрыв)
#define ACT_DBG_U_U_SEN					52	// Считать значение напряжения затвора в регистр DBG [mV]
#define ACT_DBG_U_I_SEN					53	// Считать значение тока в цепи затвора в регистр DBG [mA]
// Источник тока
#define ACT_DBG_I_I_SET					60	// Импульс тока затвора из регистра DBG [mA]
#define ACT_DBG_I_START					61	// Подача тока на нагрузку (регистр DBG = 1 -> ток через нагрузку, DBG = 0 -> ток через внутренний транзистор)
#define ACT_DBG_I_I_GATE				62	// Считать значение тока в цепи затвора в регистр DBG [mA]
#define ACT_DBG_I_U_CUTOFF				63	// Установить значение напряжения отсечки из регистра DBG [mV]
#define ACT_DBG_I_U_NEGATIVE			64	// Установить значение отрицательного напряжения смещения из регистра DBG [mV]
#define ACT_DBG_I_TEST_PULSE			65	// Тестовый импульс тока из регистра DBG [mA] на нагрузку, длительность 20 мкс
//
#define ACT_VGS_START					100	// Старт измерения порогового напряжения затвора
#define ACT_QG_START					101	// Старт измерения заряда затвора
#define ACT_STOP_PROCESS				102	// Стоп измерения

#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования
// -----------------------------

// Регистры
// Сохраняемые регистры
#define REG_DAC_OUTPUT_LIMIT_VALUE		1	// Ограничение выхода ЦАП (0 - 4095)
#define REG_REGULATOR_QI_MAX			2	// Ограничение уровня интегральной составляющей
#define REG_SCOPE_STEP					3	// Шаг сохранения оцифрованных значений
#define REG_DAC_OFFSET					4	// Смещение сигнала с ЦАП
//
#define REG_ADC_I_I_GATE_K				10	// Коэффициент преобразования кода АЦП в ток
#define REG_ADC_U_I_SEN_K				11	// Коэффициент преобразования кода АЦП в ток
#define REG_ADC_U_U_SEN_K				12	// Коэффициент преобразования кода АЦП в напряжение
#define REG_DAC_I_I_K					13	// Коэффициент преобразования тока в код ЦАП
#define REG_DAC_U_U_K					14	// Коэффициент преобразования напряжения в код ЦАП
#define REG_EXT_DAC_I_CUTOFF_K			15	// Коэффициент преобразования напряжения в код внешнего ЦАП
#define REG_EXT_DAC_I_NEGATIVE_K		16	// Коэффициент преобразования напряжения в код внешнего ЦАП
//
#define REG_ADC_I_I_GATE_P2				20	// Коэффициент тонкой подстройки Р2 х1е6
#define REG_ADC_I_I_GATE_P1				21	// Коэффициент тонкой подстройки Р1 x1000
#define REG_ADC_I_I_GATE_P0				22	// Смещение тонкой подстройки Р0
//
#define REG_ADC_U_I_SEN_P2				23	// Коэффициент тонкой подстройки Р2 х1е6
#define REG_ADC_U_I_SEN_P1				24	// Коэффициент тонкой подстройки Р1 x1000
#define REG_ADC_U_I_SEN_P0				25	// Смещение тонкой подстройки Р0
//
#define REG_ADC_U_U_SEN_P2				26	// Коэффициент тонкой подстройки Р2 х1е6
#define REG_ADC_U_U_SEN_P1				27	// Коэффициент тонкой подстройки Р1 x1000
#define REG_ADC_U_U_SEN_P0				28	// Смещение тонкой подстройки Р0
//
#define REG_DAC_I_I_P1					30	// Коэффициент тонкой подстройки Р1 x1000
#define REG_DAC_I_I_P0					31	// Смещение тонкой подстройки Р0
//
#define REG_DAC_U_U_P1					33	// Коэффициент тонкой подстройки Р1 x1000
#define REG_DAC_U_U_P0					34	// Смещение тонкой подстройки Р0
//
#define REG_EXT_DAC_I_CUTOFF_P1			36	// Коэффициент тонкой подстройки Р1 x1000
#define REG_EXT_DAC_I_CUTOFF_P0			37	// Смещение тонкой подстройки Р0
//
#define REG_EXT_DAC_I_NEGATIVE_P1		39	// Коэффициент тонкой подстройки Р1 x1000
#define REG_EXT_DAC_I_NEGATIVE_P0		40	// Смещение тонкой подстройки Р0
//
#define REG_REGULATOR_Kp				50	// Пропорциональный коэффициент регулятора
#define REG_REGULATOR_Ki				51	// Интегральный коэффициент регулятора
#define REG_REGULATOR_TF_Ki				52	// Коэффициент подстройки значения Ki (в ед. (dKi / dU) * 1000)

// Несохраняемые регистры чтения-записи
//
// Регистры измерения VGS
#define REG_U_I_TRIG					128	// Задание триггера тока [мА]
#define REG_U_T_UCONSTANT				129	// Длительность полки напряжения [мс]
#define REG_U_T_UFRONT					130	// Длительность фронта напряжения [мс]
#define	REG_U_UMAX						131 // Макс значение фронта напряжения [мВ]
// Регистры измерения QG
#define REG_I_U_CUTOFF					135	// Ограничение напряжения затвора [мВ]
#define REG_I_U_NEGATIVE				136	// Отрицательное напряжение, приложенное к затвору [мВ]
#define REG_I_I_SET						137	// Ток затвора [мА]
#define REG_I_T_CURRENT					138	// Длительность тока затвора [мкс]
//
#define REG_DBG							150	// Регистр режима Отладки
#define REG_REGULATOR_LOGGING			151	// Логгирование работы регулятора
#define REG_REGULATOR_DEBUG				152	// Режим отладки регулятора (1 = параметрическое формирование, 0 = ПИ регулятор)

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_SUB_STATE					198	// Регистр вспомогательного состояния
//
#define REG_U_VGS						200	// Измеренное значение VGS [мВ]
#define REG_I_QG						201	// Измеренное значение QG [нКл]
#define REG_I_T_IGATE					202 // Измеренная длительность импульса тока [мкс]
#define REG_I_AVERAGE_IGATE				203 // Усредненное измеренное значение тока [мА]
// -----------------------------
#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record


// Operation results
#define OPRESULT_NONE					0	// No information or not finished
#define OPRESULT_OK						1	// Operation was successful
#define OPRESULT_FAIL					2	// Operation failed

//  Fault and disable codes
#define DF_NONE							0

// Problem
#define PROBLEM_NONE					0
#define PROBLEM_BATTERY					1

//  Warning
#define WARNING_NONE					0

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ

// Endpoints
#define EP_U_U_FORM						1
#define	EP_U_U_MEAS_FORM				2
#define EP_REGULATOR_OUTPUT				3
#define EP_REGULATOR_ERR				4
#define EP_U_DAC_RAW_DATA				5
#define EP_U_I_MEAS_FORM				6
#define EP_I_I_GATE_FORM				7

#endif //  __DEV_OBJ_DIC_H
