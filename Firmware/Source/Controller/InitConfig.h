#ifndef __INITCONFIG_H
#define __INITCONFIG_H

#include "stdinc.h"

//Functions
//
Boolean INITCFG_SysClk();
void INITCFG_EI();
void INITCFG_IO();
void INITCFG_UART();
void INITCFG_ADC();
void INITCFG_Timer7();
void INITCFG_Timer3();
void INITCFG_WatchDog();
void INITCFG_ConfigCAN(Int16U NodeID);

#endif //__INITCONFIG_H
