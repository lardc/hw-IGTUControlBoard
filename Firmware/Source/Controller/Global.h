#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "SysConfig.h"

// Definitions
// 
#define	SCCI_TIMEOUT_TICKS						1000				// Таймаут интерфейса SCCI (в мс)
#define	BCCIM_TIMEOUT_TICKS						200					// Таймаут протоколоа мастер BCCI (в мс)
#define EP_WRITE_COUNT							0					// Количество массивов для записи

#define EP_COUNT								0					// Количество массивов для чтения
#define FEP_COUNT								4					// Количество массивов для чтения типа float
#define ENABLE_LOCKING							FALSE				// Защита NV регистров паролем
#define VGS_VOLTAGE_MAX							11					// Максимальное измеряемое напряжение, В
#define VGS_VOLTAGE_MIN							1					// Минимальное измеряемое напряжение, В
#define VGS_CURRENT_MAX							210					// Максимальный измеряемый ток, мА
#define MEASURE_IGES_CURRENT_MIN				1e-9				// Минимальный измеряемый токутечки, mА
#define MEASURE_IGES_CURRENT_MAX				(20 * 10e-6)		// Максимальный измеряемый токутечки, mА
#define DAC_MAX_VAL								0x0FFF				// Максимальное значение ЦАП
#define EXT_DAC_A								0x0					// Выбор канала A
#define EXT_DAC_B								0x8000				// Выбор канала B
#define QG_CURRENT_SAMPLE_TIME					0.19				// мкс
#define V_I_R0_MAX								1.0f				// мА
#define V_I_R1_MAX								20.0f				// мА
#define V_I_R2_MAX								200.0f				// мА
#define PAU_CODE_RANGE0							0
#define PAU_CODE_RANGE1							1
#define PAU_CODE_RANGE2							2
#define POWER_ON_TIMEOUT						5000				// Таймаут включения, мс

// Размер массивов EP
#define VALUES_x_SIZE							1000

// Временные параметры
#define TIME_LED_BLINK							500		// Мигание светодиодом (в мс)

#endif //  __GLOBAL_H
