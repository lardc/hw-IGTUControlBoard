#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Включение блока
#define ACT_DISABLE_POWER				2	// Выключение блока
#define ACT_CLR_FAULT					3	// Очистка всех fault
#define ACT_CLR_WARNING					4	// Очистка всех warning

// Отладочные команды
// Источник напряжения
#define ACT_DBG_V_V_SET					50	// Установить значение напряжения затвора из регистра DBG [mV]
#define ACT_DBG_V_SHORT					51	// Закоротить выход источника напряжения (регистр DBG = 1 -> закоротка, DBG = 0 -> разрыв)
#define ACT_DBG_V_V_SEN					52	// Считать значение напряжения затвора в регистр DBG [mV]
#define ACT_DBG_V_C_SEN					53	// Считать значение тока в цепи затвора в регистр DBG [mA]
// Источник тока
#define ACT_DBG_C_C_SET					60	// Импульс тока затвора из регистра DBG [mA]
#define ACT_DBG_C_START					61	// Подача тока на нагрузку (регистр DBG = 1 -> ток через нагрузку, DBG = 0 -> ток через внутренний транзистор)
#define ACT_DBG_C_C_SEN					62	// Считать значение тока в цепи затвора в регистр DBG [mA]
#define ACT_DBG_C_V_CUTOFF				63	// Установить значение напряжения отсечки из регистра DBG [mV]
#define ACT_DBG_C_V_NEGATIVE			64	// Установить значение отрицательного напряжения смещения из регистра DBG [mV]
#define ACT_DBG_C_TEST_PULSE			65	// Тестовый импульс тока из регистра DBG [mA] на нагрузку, длительность 20 мкс
//
// Команды самодиагностики
#define ACT_DIAG_V						90	// Старт диагностики источника напряжения
#define ACT_DIAG_C						91	// Старт диагностики источника тока
//
// Команды измерений
#define ACT_VGS_START					100	// Старт измерения порогового напряжения затвора
#define ACT_QG_START					101	// Старт измерения заряда затвора
#define ACT_IGES_START					102	// Старт измерения тока утечки затвор-эммитер
#define ACT_STOP_PROCESS				103	// Стоп измерения
//
#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию
//
#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования
// -----------------------------

// Регистры
// Сохраняемые регистры
#define REG_DAC_OUTPUT_LIMIT_VALUE		1	// Ограничение выхода ЦАП (0 - 4095)
#define REG_REGULATOR_QI_MAX			2	// Ограничение уровня интегральной составляющей
#define REG_SCOPE_STEP					3	// Шаг сохранения оцифрованных значений
#define REG_DAC_OFFSET					4	// Смещение сигнала с ЦАП
#define REG_V_C_SENS_THRESHOLD			5	// Порог переключения диапазонов измерения тока
#define REG_PAU_CAN_ID					6	// CAN ID PAU
//
#define REG_ADC_C_C_SEN_K				10	// Коэффициент преобразования кода АЦП в ток
#define REG_ADC_V_C_SEN_K				11	// Коэффициент преобразования кода АЦП в ток
#define REG_ADC_V_V_SEN_K				12	// Коэффициент преобразования кода АЦП в напряжение
#define REG_DAC_C_C_SET_K				13	// Коэффициент преобразования тока в код ЦАП
#define REG_DAC_V_V_SET_K				14	// Коэффициент преобразования напряжения в код ЦАП
#define REG_C_V_CUTOFF_K				15	// Коэффициент преобразования напряжения в код внешнего ЦАП
#define REG_C_V_NEGATIVE_K				16	// Коэффициент преобразования напряжения в код внешнего ЦАП
#define REG_ADC_POT_SEN_K				17	// Коэффициент преобразования кода АЦП в напряжение
#define REG_ADC_C_V_SEN_K				18	// Коэффициент преобразования кода АЦП в напряжение
//
#define REG_ADC_C_C_SEN_P2				20	// Коэффициент тонкой подстройки Р2
#define REG_ADC_C_C_SEN_P1				21	// Коэффициент тонкой подстройки Р1
#define REG_ADC_C_C_SEN_P0				22	// Смещение тонкой подстройки Р0
//
#define REG_ADC_V_C_SEN_P2				23	// Коэффициент тонкой подстройки Р2
#define REG_ADC_V_C_SEN_P1				24	// Коэффициент тонкой подстройки Р1
#define REG_ADC_V_C_SEN_P0				25	// Смещение тонкой подстройки Р0
//
#define REG_ADC_V_V_SEN_P2				26	// Коэффициент тонкой подстройки Р2
#define REG_ADC_V_V_SEN_P1				27	// Коэффициент тонкой подстройки Р1
#define REG_ADC_V_V_SEN_P0				28	// Смещение тонкой подстройки Р0
//
#define REG_ADC_POT_SEN_P2				29	// Коэффициент тонкой подстройки Р2
#define REG_ADC_POT_SEN_P1				30	// Коэффициент тонкой подстройки Р1
#define REG_ADC_POT_SEN_P0				31	// Смещение тонкой подстройки Р0
//
#define REG_ADC_C_V_SEN_P2				32	// Коэффициент тонкой подстройки Р2
#define REG_ADC_C_V_SEN_P1				33	// Коэффициент тонкой подстройки Р1
#define REG_ADC_C_V_SEN_P0				34	// Смещение тонкой подстройки Р0
//
#define REG_DAC_C_C_SET_P1				35	// Коэффициент тонкой подстройки Р1
#define REG_DAC_C_C_SET_P0				36	// Смещение тонкой подстройки Р0
//
#define REG_DAC_V_V_SET_P1				37	// Коэффициент тонкой подстройки Р1
#define REG_DAC_V_V_SET_P0				38	// Смещение тонкой подстройки Р0
//
#define REG_C_V_CUTOFF_P1				39	// Коэффициент тонкой подстройки Р1
#define REG_C_V_CUTOFF_P0				40	// Смещение тонкой подстройки Р0
//
#define REG_C_V_NEGATIVE_P1				41	// Коэффициент тонкой подстройки Р1
#define REG_C_V_NEGATIVE_P0				42	// Смещение тонкой подстройки Р0
//
#define REG_REGULATOR_Kp				50	// Пропорциональный коэффициент регулятора
#define REG_REGULATOR_Ki				51	// Интегральный коэффициент регулятора

#define REG_PAU_SNC_DELAY				60	// Задержка синхронизации PAU с момента выхода на полку импульса [ms]
#define REG_PAU_EMULATED				61	// Эмуляция работы PAU
//
// Несохраняемые регистры чтения-записи
//
// Регистры измерения VGS
#define REG_VGS_C_TRIG					128	// Задание триггера тока [мА]
#define REG_VGS_T_V_CONSTANT			129	// Длительность полки напряжения [мс]
#define REG_VGS_T_V_FRONT				130	// Длительность фронта напряжения [мс]
#define	REG_VGS_V_MAX					131 // Макс значение напряжения [мВ]
// Регистры измерения QG
#define REG_QG_V_CUTOFF					135	// Ограничение напряжения затвора [мВ]
#define REG_QG_V_NEGATIVE				136	// Отрицательное напряжение, приложенное к затвору [мВ]
#define REG_QG_C_SET					137	// Ток затвора [мА]
#define REG_QG_T_CURRENT				138	// Длительность тока затвора [мкс]
#define REG_QG_C_POWER					139	// Силовой ток коллектор-эммитер (сток-исток) [А]
#define REG_QG_V_POWER					140	// Напряжение коллектор-эммитер (сток-исток) [В]
#define REG_QG_C_THRESHOLD				141	// Порог срабатывания триггера тока (в % от REG_QG_C_SET) [%]
// Регистры измерения IGES
#define REG_IGES_V						145	// Полка напряжения на затворе [мВ]
#define REG_IGES_T_V_CONSTANT			146	// Длительность полки напряжения [мс]
#define REG_IGES_T_V_FRONT				147	// Длительность фронта напряжения [мс]
//
#define REG_DBG							150	// Регистр режима Отладки
#define REG_REGULATOR_LOGGING			151	// Логгирование работы регулятора
#define REG_REGULATOR_DEBUG				152	// Режим отладки регулятора (1 = параметрическое формирование, 0 = ПИ регулятор)
#define REG_DIAG_RESULT					153	// Регистр результата самодиагностики

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_SUB_STATE					198	// Регистр вспомогательного состояния

// Результаты измерения
#define REG_VGS							200	// Измеренное значение VGS [мВ]
//
#define REG_QG							205	// Измеренное значение QG [нКл]
#define REG_QG_T						206 // Измеренная длительность импульса тока [мкс]
#define REG_QG_C						207 // Усредненное измеренное значение тока [мА]
//
#define REG_IGES						210	// Измеренное значение IGES [нА]
//

#define REG_PAU_ERROR_CODE				220	// Ошибка интерфейса PAU: код ошибки
#define REG_PAU_FUNCTION				221	// Ошибка интерфейса PAU: код функции
#define REG_PAU_EXT_DATA				222	// Ошибка интерфейса PAU: расширенная информация
// -----------------------------
#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record

// PAU commands
#define ACT_PAU_FAULT_CLEAR				3	// Очистка fault
#define ACT_PAU_WARNING_CLEAR			4	// Очистка warning
//
#define ACT_PAU_PULSE_CONFIG			100	// Конфигурация PAU
// -----------------------------

// Регистры PAU
#define REG_PAU_CHANNEL					128	// Выбор канала измерения PAU
#define REG_PAU_RANGE					129	// Диапазон измерения тока (мА)
#define REG_PAU_MEASUREMENT_TIME		130	// Время измерения (мс)
//
#define REG_PAU_RESULT_CURRENT			200	// Измеренное значение тока
//
#define REG_PAU_DEV_STATE				192	// Состояние блока
#define REG_PAU_FAULT_REASON			193
#define REG_PAU_DISABLE_REASON			194
#define REG_PAU_WARNING					195
#define REG_PAU_PROBLEM					196
// -----------------------------

// Operation results
#define OPRESULT_NONE					0	// No information or not finished
#define OPRESULT_OK						1	// Operation was successful
#define OPRESULT_FAIL					2	// Operation failed

//  Fault and disable codes
#define DF_NONE							0

// Problem
#define PROBLEM_NONE					0
#define PROBLEM_CURRENT_NOT_REACHED		1
#define PROBLEM_NEGATIVE_CURRENT		2
#define PROBLEM_PAU_REQUEST_ERROR		3
#define PROBLEM_TOCUHP_REQUEST_ERROR	4

//  Warning
#define WARNING_NONE					0

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ

// Endpoints
#define EP_V_V_FORM						1
#define	EP_V_V_MEAS_FORM				2
#define EP_REGULATOR_ERR				3
#define EP_V_C_MEAS_FORM				4
#define EP_C_C_FORM						5

#endif //  __DEV_OBJ_DIC_H
