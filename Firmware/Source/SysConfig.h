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
#define TIMER6_uS						1			// в мкс
#define TIMER4_uS						2			// в мкс
// ----------------------------------------------

// CAN
#define CAN_BAUDRATE					1000000		// Битрейт CAN
// ----------------------------------------------

// ADC
#define ADC1_I_I_SEN_CHANNEL			1				// АЦП1 номер канала тока (источник C)
#define ADC1_I_V_SEN_CHANNEL			2				// АЦП1 номер канала напряжения (источник C)
#define ADC1_V_I_LOW_SEN_CHANNEL		3				// АЦП1 номер канала напряжения (источник V)
#define ADC1_V_I_HIGH_SEN_CHANNEL		4				// АЦП1 номер канала тока (источник V)
#define ADC3_POT_CHANNEL				1				// АЦП3 номер канала напряжения POT
#define ADC_V_DMA_BUFF_SIZE				5				// Количество данных DMA для измерения Vgs/Iges
#define ADC_DMA_BUFF_SIZE_QG			3000			// Количество данных DMA для измерения Qg
// ----------------------------------------------

// SPI
#define SPI1_BAUDRATE_BITS				0x1				// Биты задания битрейта SPI
#define SPI1_LSB_FIRST					false			// Передача младшим битов вперед
// ----------------------------------------------

#endif // __SYSCONFIG_H
