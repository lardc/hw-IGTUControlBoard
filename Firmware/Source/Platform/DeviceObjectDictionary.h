#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Включение блока
#define ACT_DISABLE_POWER				2	// Выключение блока
#define ACT_CLR_FAULT					3	// Очистка всех fault
#define ACT_CLR_WARNING					4	// Очистка всех warning

// Отладочные команды
// Источник напряжения
#define ACT_DBG_V_V_SET					50	// Установить значение напряжения затвора из регистра DBG (mV)
#define ACT_DBG_V_SHORT					51	// Закоротить выход источника напряжения (регистр DBG = 1 -> закоротка, DBG = 0 -> разрыв)
#define ACT_DBG_PAU_SHORT				52	// Управление шунтированием PAU
#define ACT_DBG_I_LIM_LOW_TEST			53	// Тест на проверку ограничения тока младшего диапазона
#define ACT_DBG_I_LIM_HIGH_TEST			54	// Тест на проверку ограничения тока старшего диапазона
// Источник тока
#define ACT_DBG_I_V_CUTOFF				55	// Установить значение напряжения отсечки из регистра DBG (mV)
#define ACT_DBG_I_V_NEGATIVE			56	// Установить значение отрицательного напряжения смещения из регистра DBG (mV)
#define ACT_DBG_I_TEST_PULSE			57	// Тестовый импульс тока из регистра DBG (mA) на нагрузку, длительность 20 мкс
//
#define ACT_DBG_SYNC_PAU				60	// Тест синхронизации PAU
#define ACT_DBG_SYNC_TOCUHP				61	// Тест синхронизации TOCU HP
#define ACT_DBG_SYNC_OSC				62	// Тест синхронизации осциллографа
#define ACT_DBG_EXT_IND					63	// Тест внешнего индикатора
#define ACT_DBG_SWITCH_MUX				64	// Тест переключения выходного мультиплексора на реле
#define ACT_DBG_SWITCH_TO_DIAG			65	// Тест переключения в режим самодиагностики
#define ACT_DBG_PROTECTION				66	// Тест управления системой защиты

// Команды для калибровки
#define ACT_CAL_V						70	// Калибровка источника напряжения
#define ACT_CAL_I						71	// Калибровка источника тока

// Команды измерений
#define ACT_START_VGS					100	// Старт измерения порогового напряжения затвора
#define ACT_START_QG					101	// Старт измерения заряда затвора
#define ACT_START_IGES					102	// Старт измерения тока утечки затвор-эммитер
#define ACT_START_RES					103	// Старт измерения сопротивления
#define ACT_STOP_PROCESS				104	// Стоп измерения
//
#define ACT_START_SELF_TEST				110	// Запуск самотестирования
//
#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию
//
#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования
// -----------------------------

// Регистры
// Сохраняемые регистры
#define REG_V_ADC_TO_V_P2				0	// Коэффициент тонкой подстройки Р2
#define REG_V_ADC_TO_V_P1				1	// Коэффициент тонкой подстройки Р1
#define REG_V_ADC_TO_V_P0				2	// Смещение тонкой подстройки Р0
#define REG_V_ADC_TO_V_K				3	// Коэффициент преобразования K
#define REG_V_ADC_TO_V_B				4	// Коэффициент преобразования B
//
#define REG_V_ADC_TO_I_R0_P2			5	// Коэффициент тонкой подстройки Р2
#define REG_V_ADC_TO_I_R0_P1			6	// Коэффициент тонкой подстройки Р1
#define REG_V_ADC_TO_I_R0_P0			7	// Смещение тонкой подстройки Р0
#define REG_V_ADC_TO_I_R0_K				8	// Коэффициент преобразования K
#define REG_V_ADC_TO_I_R0_B				9	// Коэффициент преобразования B
//
#define REG_V_ADC_TO_I_R1_P2			10	// Коэффициент тонкой подстройки Р2
#define REG_V_ADC_TO_I_R1_P1			11	// Коэффициент тонкой подстройки Р1
#define REG_V_ADC_TO_I_R1_P0			12	// Смещение тонкой подстройки Р0
#define REG_V_ADC_TO_I_R1_K				13	// Коэффициент преобразования K
#define REG_V_ADC_TO_I_R1_B				14	// Коэффициент преобразования B
//
#define REG_V_ADC_TO_I_R2_P2			15	// Коэффициент тонкой подстройки Р2
#define REG_V_ADC_TO_I_R2_P1			16	// Коэффициент тонкой подстройки Р1
#define REG_V_ADC_TO_I_R2_P0			17	// Смещение тонкой подстройки Р0
#define REG_V_ADC_TO_I_R2_K				18	// Коэффициент преобразования K
#define REG_V_ADC_TO_I_R2_B				19	// Коэффициент преобразования B
//
#define REG_V_V_TO_DAC_P2				20	// Коэффициент тонкой подстройки Р2
#define REG_V_V_TO_DAC_P1				21	// Коэффициент тонкой подстройки Р1
#define REG_V_V_TO_DAC_P0				22	// Смещение тонкой подстройки Р0
#define REG_V_V_TO_DAC_K				23	// Коэффициент преобразования K
#define REG_V_V_TO_DAC_B				24	// Коэффициент преобразования B
//
#define REG_I_I_TO_DAC_P2				25	// Коэффициент тонкой подстройки Р2
#define REG_I_I_TO_DAC_P1				26	// Коэффициент тонкой подстройки Р1
#define REG_I_I_TO_DAC_P0				27	// Смещение тонкой подстройки Р0
#define REG_I_I_TO_DAC_K				28	// Коэффициент преобразования K
#define REG_I_I_TO_DAC_B				29	// Коэффициент преобразования B
//
#define REG_I_VC_TO_DAC_P2				30	// Коэффициент тонкой подстройки Р2
#define REG_I_VC_TO_DAC_P1				31	// Коэффициент тонкой подстройки Р1
#define REG_I_VC_TO_DAC_P0				32	// Смещение тонкой подстройки Р0
#define REG_I_VC_TO_DAC_K				33	// Коэффициент преобразования K
#define REG_I_VC_TO_DAC_B				34	// Коэффициент преобразования B
//
#define REG_I_VN_TO_DAC_P2				35	// Коэффициент тонкой подстройки Р2
#define REG_I_VN_TO_DAC_P1				36	// Коэффициент тонкой подстройки Р1
#define REG_I_VN_TO_DAC_P0				37	// Смещение тонкой подстройки Р0
#define REG_I_VN_TO_DAC_K				38	// Коэффициент преобразования K
#define REG_I_VN_TO_DAC_B				39	// Коэффициент преобразования B
//
#define REG_I_ADC_TO_VC_P2				40	// Коэффициент тонкой подстройки Р2
#define REG_I_ADC_TO_VC_P1				41	// Коэффициент тонкой подстройки Р1
#define REG_I_ADC_TO_VC_P0				42	// Смещение тонкой подстройки Р0
#define REG_I_ADC_TO_VC_K				43	// Коэффициент преобразования K
#define REG_I_ADC_TO_VC_B				44	// Коэффициент преобразования B
#define REG_I_ADC_TO_V_ZERO_OFFSET		45	// Смещение для выставления нуля
//
#define REG_I_ADC_TO_I_P2				46	// Коэффициент тонкой подстройки Р2
#define REG_I_ADC_TO_I_P1				47	// Коэффициент тонкой подстройки Р1
#define REG_I_ADC_TO_I_P0				48	// Смещение тонкой подстройки Р0
#define REG_I_ADC_TO_I_K				49	// Коэффициент преобразования K
#define REG_I_ADC_TO_I_B				50	// Коэффициент преобразования B
//
#define REG_REGULATOR_Kp				51	// Пропорциональный коэффициент регулятора
#define REG_REGULATOR_Ki				52	// Интегральный коэффициент регулятора
#define REG_REGULATOR_QI_MAX			53	// Ограничение уровня интегральной составляющей
#define REG_REGULATOR_ERR_MAX			54	// Уровень ошибки регулятора для Following Error (%)
#define REG_REGULATOR_FE_COUNTER		55	// Счетчик Following Error
#define REG_REGULATOR_PARAMETRIC		56	// Режим параметрического формирования
//
#define REG_VGS_FAST_RATE				57	// Скорость нарастания напряжения Vg (мВ/мкс)
#define REG_VGS_SLOW_RATE				58	// Скорость нарастания напряжения Vg (мВ/мкс)
#define REG_VGS_dI_TRIG					59	// (%)
//
#define REG_IGES_V_RATE					60	// Скорость нарастания напряжения, (В/мкс)
#define REG_IGES_SAMPLES_NUMBER			61	// Количество семплов при измерении Iges
//
#define REG_PAU_CAN_ID					62	// CAN ID PAU
#define REG_PAU_EMULATED				63	// Эмуляция работы PAU
//
#define REG_TOCUHP0_CAN_ID				64	// CAN ID TOCUHP 0
#define REG_TOCUHP1_CAN_ID				65	// CAN ID TOCUHP 1
#define REG_TOCUHP2_CAN_ID				66	// CAN ID TOCUHP 2
#define REG_TOCUHP0_BITS				67	// Разрядность TOCUHP 0
#define REG_TOCUHP1_BITS				68	// Разрядность TOCUHP 1
#define REG_TOCUHP2_BITS				69	// Разрядность TOCUHP 2
#define REG_TOCUHP0_RES_PER_BIT0		70	// Сопротивление одного бита блока TOCUHP 0 (Ом)
#define REG_TOCUHP1_RES_PER_BIT0		71	// Сопротивление одного бита блока TOCUHP 1 (Ом)
#define REG_TOCUHP2_RES_PER_BIT0		72	// Сопротивление одного бита блока TOCUHP 2 (Ом)
#define REG_TOCUHP_EMULATED				73	// Эмуляция работы TOCUHP
//
#define REG_DAC_OUTPUT_LIMIT_VALUE		74	// Ограничение выхода ЦАП (0 - 4095)
#define REG_SCOPE_STEP					75	// Шаг сохранения оцифрованных значений
#define REG_MUTE_SAFETY					76	// Отключение проверки контакта безопасности
#define REG_ST_CHECK_ERROR				77	// Допустимая ошибка результата самотестирования
#define REG_DUT_CONN_CHECK				78	// Определение подключенного прибора
//
#define REG_I_ADC_TO_VN_P2				79	// Коэффициент тонкой подстройки Р2
#define REG_I_ADC_TO_VN_P1				80	// Коэффициент тонкой подстройки Р1
#define REG_I_ADC_TO_VN_P0				81	// Смещение тонкой подстройки Р0
#define REG_I_ADC_TO_VN_K				82	// Коэффициент преобразования K
#define REG_I_ADC_TO_VN_B				83	// Коэффициент преобразования B


// Несохраняемые регистры чтения-записи
//
// Регистры измерения VGS
#define REG_VGS_I_TRIG					128	// Задание триггера тока (мА)
#define	REG_VGS_V_MAX					129	// Значение напряжения ограничения (В)

// Регистры измерения QG
#define REG_QG_V_CUTOFF					130	// Ограничение напряжения затвора (В)
#define REG_QG_V_NEGATIVE				131	// Отрицательное напряжение, приложенное к затвору (В)
#define REG_QG_I						132	// Ток затвора (мА)
#define REG_QG_I_DURATION				133	// Длительность тока затвора (мкс)
#define REG_QG_I_POWER					134	// Силовой ток коллектор-эммитер (сток-исток) (А)
#define REG_QG_V_POWER					135	// Напряжение коллектор-эммитер (сток-исток) (В)

// Регистры измерения IGES
#define REG_IGES_V						136	// Полка напряжения на затворе (В)
#define REG_IGES_RANGE					137	// Диапазон измерения тока утечки (0 - 20нА, 1 - 200нА, 2 - 2мкА)

// Регистры режимов калибровки
#define REG_SERTIFICATION				139	// 0 - обычный режим, 1 - режим аттестации
#define REG_CAL_VP						140	// Задание положительного напряжения при калибровке, В
#define REG_CAL_VN						141	// Задание отрицательного напряжения при калибровке, В
#define REG_CAL_I						142	// Задание тока при калибровке, мА


#define REG_DBG							150	// Отладочный регистр
//

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_SELF_TEST_OP_RESULT			198	// Регистр результата самотестирования
#define REG_SUB_STATE					199	// Регистр вспомогательного состояния
//

// Результаты измерения
#define REG_VGS_RESULT					200	// Измеренное значение VGS, (В)
#define REG_VGS_I_RESULT				201	// Достигнутый ток, при измерении Vgs, (мА)
#define REG_QG_RESULT					202	// Измеренное значение QG (нКл)
#define REG_QG_I_RESULT					203 // Усредненное измеренное значение тока (мА)
#define REG_IGES_RESULT					204	// Измеренное значение IGES (нА)
#define REG_RES_RESULT					205	// Измеренное значение сопротивления (Ом)
//
#define REG_CAL_V_RESULT				210
#define REG_CAL_VN_RESULT				211
#define REG_CAL_I_RESULT				212

#define REG_EXT_UNIT_ERROR_CODE			220	// Ошибка интерфейса PAU: код ошибки
#define REG_EXT_UNIT_FUNCTION			221	// Ошибка интерфейса PAU: код функции
#define REG_EXT_UNIT_EXT_DATA			222	// Ошибка интерфейса PAU: расширенная информация
// -----------------------------
#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record
// -----------------------------

// PAU commands
#define ACT_PAU_FAULT_CLEAR				3	// Очистка fault
#define ACT_PAU_WARNING_CLEAR			4	// Очистка warning
//
#define ACT_PAU_PULSE_CONFIG			100	// Конфигурация PAU

// PAU registers
#define REG_PAU_CHANNEL					128	// Выбор канала измерения PAU
#define REG_PAU_RANGE					129	// Диапазон измерения тока (мА)
#define REG_PAU_SAMPLES_NUMBER			130	// Количество точек измерения
#define REG_PAU_RESULT_CURRENT			200	// Измеренное значение тока
//
#define REG_PAU_DEV_STATE				192	// Состояние блока
#define REG_PAU_FAULT_REASON			193
#define REG_PAU_DISABLE_REASON			194
#define REG_PAU_WARNING					195
#define REG_PAU_PROBLEM					196
// -----------------------------

// TOCUHP commands
#define ACT_TOCUHP_ENABLE_POWER			1	// Переход в состояние ожидания
#define ACT_TOCUHP_DISABLE_POWER		2	// Отключение блока
#define ACT_TOCUHP_FAULT_CLEAR			3	// Очистка fault
#define ACT_TOCUHP_WARNING_CLEAR		4	// Очистка warning
#define ACT_TOCUHP_VOLTAGE_CONFIG		100	// Конфигурация напряжения блока
#define ACT_TOCUHP_PULSE_CONFIG			101	// Конфигурация блока под требуемый ток
// TOCUHP registers
#define REG_TOCUHP_VOLTAGE_SETPOINT		128	// Значение задания напряжения (В)
#define REG_TOCUHP_GATE_REGISTER		129	// Значение-маска конфигурации затворов
//
#define REG_TOCUHP_DEV_STATE			192	// Состояние блока
#define REG_TOCUHP_FAULT_REASON			193
#define REG_TOCUHP_DISABLE_REASON		194
#define REG_TOCUHP_WARNING				195
#define REG_TOCUHP_PROBLEM				196
#define REG_TOCUHP_OP_RESULT			197
// -----------------------------

// Operation results
#define OPRESULT_NONE					0	// No information or not finished
#define OPRESULT_OK						1	// Operation was successful
#define OPRESULT_FAIL					2	// Operation failed

//  Fault and disable codes
#define DF_NONE							0
#define DF_FOLLOWING_ERROR				1
#define DF_PAU_INTERFACE				2
#define DF_PAU_WRONG_STATE				3
#define DF_PAU_SYNC_TIMEOUT				4
#define DF_TOCUHP_INTERFACE				5
#define DF_TOCUHP_WRONG_STATE			6
#define DF_TOCUHP_PWR_ON_TIMEOUT		7
#define DF_POWER_CURRENT				8
#define DF_ST_V_SOURCE_ERROR			9
#define DF_ST_I_SOURCE_ERROR			10

// Problem
#define PROBLEM_NONE					0
#define PROBLEM_FORCED_STOP				1
#define PROBLEM_SAFETY_VIOLATION		2
#define PROBLEM_CURRENT_NOT_REACHED		3
#define PROBLEM_SHORT					4
#define PROBLEM_DUT_NOT_FOUND			5

//  Warning
#define WARNING_NONE					0
#define WARNING_OUT_OF_RANGE			1

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ
#define ERR_WRONG_PARAMS				5	// Заданы некорректные параметры

// Endpoints
#define EP_VOLTAGE						1
#define	EP_CURRENT						2
#define EP_REGULATOR_ERR				3
#define EP_REGULATOR_OUTPUT				4

#endif //  __DEV_OBJ_DIC_H
