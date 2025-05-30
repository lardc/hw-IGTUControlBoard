#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Переход в состояние ожидания
#define ACT_DISABLE_POWER				2	// Отключение блока
#define ACT_FAULT_CLEAR					3	// Очистка fault
#define ACT_WARNING_CLEAR				4	// Очистка warning

#define ACT_DBG_EXT_INDICATION			10	// Управление внешней индикацией
#define ACT_DBG_SPI_WRITE_TWO_BYTES		11	// Запись двух байтов для отладки SPI

#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования

#define ACT_JSON_INIT_READ				341	// Инициализация начала считывания JSON
#define ACT_JSON_TO_EP					342	// Выполнить чтение шаблона JSON в EP
// -----------------------------

// Регистры
// Сохраняемые регистры
#define REG_U_ADC_REF					0	// Опорное напряжение ADC/DAC
//
#define REG_U_1_P2						1	// Коэффициент тонкой подстройки Р2
#define REG_U_1_P1						2	// Коэффициент тонкой подстройки Р1
#define REG_U_1_P0						3	// Смещение тонкой подстройки Р0
#define REG_U_1_K						4	// Коэффициент преобразования K
#define REG_U_1_B						5	// Коэффициент преобразования B
//
#define REG_U_2_P2						6	// Коэффициент тонкой подстройки Р2
#define REG_U_2_P1						7	// Коэффициент тонкой подстройки Р1
#define REG_U_2_P0						8	// Смещение тонкой подстройки Р0
#define REG_U_2_K						9	// Коэффициент преобразования K
#define REG_U_2_B						10	// Коэффициент преобразования B
//
#define REG_I_1_P2						11	// Коэффициент тонкой подстройки Р2
#define REG_I_1_P1						12	// Коэффициент тонкой подстройки Р1
#define REG_I_1_P0						13	// Смещение тонкой подстройки Р0
#define REG_I_1_K						14	// Коэффициент преобразования K
#define REG_I_1_B						15	// Коэффициент преобразования B
#define REG_I_1_RSH						16	// Сопротивление шунта
//
#define REG_I_2_P2						17	// Коэффициент тонкой подстройки Р2
#define REG_I_2_P1						18	// Коэффициент тонкой подстройки Р1
#define REG_I_2_P0						19	// Смещение тонкой подстройки Р0
#define REG_I_2_K						20	// Коэффициент преобразования K
#define REG_I_2_B						21	// Коэффициент преобразования B
#define REG_I_2_RSH						22	// Сопротивление шунта
//
#define REG_I_3_P2						23	// Коэффициент тонкой подстройки Р2
#define REG_I_3_P1						24	// Коэффициент тонкой подстройки Р1
#define REG_I_3_P0						25	// Смещение тонкой подстройки Р0
#define REG_I_3_K						26	// Коэффициент преобразования K
#define REG_I_3_B						27	// Коэффициент преобразования B
#define REG_I_3_RSH						28	// Сопротивление шунта
//
#define REG_I_4_P2						29	// Коэффициент тонкой подстройки Р2
#define REG_I_4_P1						30	// Коэффициент тонкой подстройки Р1
#define REG_I_4_P0						31	// Смещение тонкой подстройки Р0
#define REG_I_4_K						32	// Коэффициент преобразования K
#define REG_I_4_B						33	// Коэффициент преобразования B
#define REG_I_4_RSH						34	// Сопротивление шунта
//
#define REG_I_5_P2						35	// Коэффициент тонкой подстройки Р2
#define REG_I_5_P1						36	// Коэффициент тонкой подстройки Р1
#define REG_I_5_P0						37	// Смещение тонкой подстройки Р0
#define REG_I_5_K						38	// Коэффициент преобразования K
#define REG_I_5_B						39	// Коэффициент преобразования B
#define REG_I_5_RSH						40	// Сопротивление шунта
//
#define REG_I_6_P2						41	// Коэффициент тонкой подстройки Р2
#define REG_I_6_P1						42	// Коэффициент тонкой подстройки Р1
#define REG_I_6_P0						43	// Смещение тонкой подстройки Р0
#define REG_I_6_K						44	// Коэффициент преобразования K
#define REG_I_6_B						45	// Коэффициент преобразования B
#define REG_I_6_RSH						46	// Сопротивление шунта
//
#define REG_I_7_P2						47	// Коэффициент тонкой подстройки Р2
#define REG_I_7_P1						48	// Коэффициент тонкой подстройки Р1
#define REG_I_7_P0						49	// Смещение тонкой подстройки Р0
#define REG_I_7_K						50	// Коэффициент преобразования K
#define REG_I_7_B						51	// Коэффициент преобразования B
#define REG_I_7_RSH						52	// Сопротивление шунта
//
#define REG_I_8_P2						53	// Коэффициент тонкой подстройки Р2
#define REG_I_8_P1						54	// Коэффициент тонкой подстройки Р1
#define REG_I_8_P0						55	// Смещение тонкой подстройки Р0
#define REG_I_8_K						56	// Коэффициент преобразования K
#define REG_I_8_B						57	// Коэффициент преобразования B
#define REG_I_8_RSH						58	// Сопротивление шунта
//
// 59
//
#define REG_CFG_NODE_ID					60	// Настройка CAN NodeID
//
#define REG_U_SET_P2					61	// Коэффициент тонкой подстройки Р2
#define REG_U_SET_P1					62	// Коэффициент тонкой подстройки Р1
#define REG_U_SET_P0					63	// Смещение тонкой подстройки Р0
#define REG_U_SET_K						64	// Коэффициент преобразования K
#define REG_U_SET_B						65	// Коэффициент преобразования B
//
// 66 - 149
//
#define REG_DBG							150	// Отладочный регистр
//
// 151 - 191
//

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_DEV_SUBSTATE				198
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

//  Warning
#define WARNING_NONE					0

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ

// EP
#define EP16_ExtInfoData				20	// Diag data drom flash

#endif //  __DEV_OBJ_DIC_H
