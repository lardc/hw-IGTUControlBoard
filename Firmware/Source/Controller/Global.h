#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "SysConfig.h"

// Definitions
// 
#define	SCCI_TIMEOUT_TICKS						1000	// Таймаут интерфейса SCCI (в мс)
#define	BCCIM_TIMEOUT_TICKS						50		// Таймаут протоколоа мастер BCCI (в мс)
#define EP_WRITE_COUNT							0		// Количество массивов для записи

#define EP_COUNT								0		// Количество массивов для чтения
#define FEP_COUNT								4		// Количество массивов для чтения типа float
#define ENABLE_LOCKING							FALSE	// Защита NV регистров паролем
#define CURRENT_PULSE_WIDTH						300		// Макс длительность импульса тока (мкс)
#define VOLTAGE_PULSE_WIDTH						12000	// Макс длительность импульса напряжения (мкс)
#define MEASURE_VGS_CURRENT_MAX					210		// Максимальный измеряемый ток, мА
#define MEASURE_IGES_CURRENT_MIN				1		// Минимальный измеряемый токутечки, нА
#define MEASURE_IGES_CURRENT_MAX				300		// Максимальный измеряемый токутечки, нА
#define DAC_MAX_VAL								0x0FFF	// Максимальное значение ЦАП
#define EXT_DAC_A								0x0		// Выбор канала A
#define EXT_DAC_B								0x8000	// Выбор канала B

// Размер массивов EP
#define VALUES_x_SIZE							1000

// Временные параметры
#define TIME_LED_BLINK							500		// Мигание светодиодом (в мс)

#endif //  __GLOBAL_H
