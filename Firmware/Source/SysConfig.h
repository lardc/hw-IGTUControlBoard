#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

// Flash loader options
#define BOOT_LOADER_VARIABLE			(*((volatile uint32_t *)0x20000000))
#define BOOT_LOADER_REQUEST				0x12345678
#define BOOT_LOADER_MAIN_PR_ADDR		0x08008000
//-----------------------------------------------

// System clock
#define SYSCLK							70000000	// Тактовая частота системной шины процессора
#define QUARTZ_FREQUENCY				20000000	// Частота кварца
// ----------------------------------------------

// USART
#define USART_BAUDRATE					115200		// Скорость USART
#define USART_FIFOlen					32			// Длина FIFO USART
// ----------------------------------------------

// Timers
#define TIMER15_uS						50			// в мкс
#define TIMER7_uS						1000		// в мкс
#define TIMER6_nS						500			// в нс
#define TIMER6_uS						((float)TIMER6_nS / 1000)
#define TIMER4_uS						2			// в мкс
// ----------------------------------------------

// CAN
#define CAN_BAUDRATE					1000000		// Битрейт CAN
// ----------------------------------------------

// ADC
#define DMA_ADC_C_C_SEN_CHANNEL			DMA1_Channel1	// Канал DMA для АЦП тока затвора (источник C)
#define ADC1_C_C_SEN_CHANNEL			1				// АЦП1 номер канала тока (источник C)
#define ADC1_C_V_SEN_CHANNEL			2				// АЦП1 номер канала напряжения (источник C)
#define ADC1_V_V_SEN_CHANNEL			3				// АЦП1 номер канала напряжения (источник V)
#define ADC1_V_C_SEN_CHANNEL			4				// АЦП1 номер канала тока (источник V)
#define ADC3_POT_CHANNEL				1				// АЦП3 номер канала напряжения POT
#define ADC_DMA_BUFF_SIZE				5				// Количество данных для DMA
// ----------------------------------------------

// SPI
#define SPI1_BAUDRATE_BITS				0x1				// Биты задания битрейта SPI
#define SPI1_LSB_FIRST					false			// Передача младшим битов вперед
// ----------------------------------------------

#endif // __SYSCONFIG_H
