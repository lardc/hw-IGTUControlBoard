#ifndef __INITCONFIG_H
#define __INITCONFIG_H

#include "stdinc.h"

//Functions
//
Boolean INITCFG_ConfigSystemClock();
void INITCFG_ConfigDAC();
void INITCFG_ConfigIO();
void INITCFG_ConfigUART();
void INITCFG_ConfigCAN();
void INITCFG_ConfigTimer7();
void INITCFG_ConfigTimer15();
void INITCFG_ConfigTimer4();
void INITCFG_ConfigWatchDog();
void INITCFG_ConfigExtInterrupt();
void INITCFG_ConfigSPI();
void INITCFG_ConfigDMA_VgsIges();
void INITCFG_ConfigADC_VgsIges(Int16U CurrentRange);
void INITCFG_ConfigDMA_Qg(Int16U DMA_DataSize);
void INITCFG_ConfigADC_QgXX(bool OnlyCurrentSample);
void INITCFG_ConfigADC_Qg_VI();
void INITCFG_ConfigADC_Qg_I();
void INITCFG_ConfigADC();
void INITCFG_ConfigDMA();
void INITCFG_ConfigTimer3(Int16U Period);
void INITCFG_ADC_SoftTrigConfig(ADC_TypeDef* ADCx);

#endif //__INITCONFIG_H
