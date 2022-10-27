// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_UShortOut(bool State)
{
	GPIO_SetState(GPIO_SHORT_OUT, !State);
}
//-----------------------------

void LL_IStart(bool State)
{
	GPIO_SetState(GPIO_I_START, State);
}
//-----------------------------

bool LL_ICompState()
{
	return GPIO_GetState(GPIO_I_COMP);
}
//-----------------------------

void LL_IISetDAC(Int16U Data)
{
	DAC_SetValueCh1(DAC1, Data);
	//DAC_SetValueCh2(DAC1, Data);
}
//-----------------------------

void LL_UUSetDAC(Int16U Data)
{
	DAC_SetValueCh2(DAC1, Data);
}
//-----------------------------

void LL_ExtDACSync(bool State)
{
	GPIO_SetState(GPIO_EXT_DAC_CS, State);
}
//-----------------------------

void LL_ExtDACLDAC(bool State)
{
	GPIO_SetState(GPIO_EXT_DAC_LDAC, State);
}
//-----------------------------

void LL_ExtDACSendData(Int16U Data)
{
	LL_ExtDACSync(false);
	SPI_WriteByte(SPI1, Data);
	LL_ExtDACSync(true);
	LL_ExtDACLDAC(false);
	DELAY_US(1);
	LL_ExtDACLDAC(true);
}
//-----------------------------
