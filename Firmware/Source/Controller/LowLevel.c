// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"
#include "ConvertUtils.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_Indication(bool State)
{
	GPIO_SetState(GPIO_INDICATION, !State);
}
//-----------------------------

void LL_OutMultiplexCurrent()
{
	GPIO_SetState(GPIO_OUT_MULTIPLEX, false);
}
//-----------------------------

void LL_OutMultiplexVoltage()
{
	GPIO_SetState(GPIO_OUT_MULTIPLEX, true);
}
//-----------------------------

void LL_SyncTOCUHP(bool State)
{
	GPIO_SetState(GPIO_SYNC_TOCUHP, State);
}
//-----------------------------

void LL_SyncOSC(bool State)
{
	GPIO_SetState(GPIO_SYNC_OSC, State);
}
//-----------------------------

void LL_SyncPAU(bool State)
{
	GPIO_SetState(GPIO_SYNC_PAU, State);
}
//-----------------------------

bool LL_SafetyState()
{
	return GPIO_GetState(GPIO_SYNC_SAFETY);
}
//-----------------------------

// V source
void LL_V_ShortOut(bool State)
{
	GPIO_SetState(GPIO_V_SHORT_OUT, !State);
}
//-----------------------------

void LL_V_ShortPAU(bool State)
{
	GPIO_SetState(GPIO_V_SHORT_PAU, !State);
}
//-----------------------------

void LL_V_VSetDAC(Int16U Data)
{
	DAC_SetValueCh2(DAC1, Data);
}
//-----------------------------

void LL_V_IsenseSetRange(bool Range)
{
	if(Range)
	{
		GPIO_SetState(GPIO_V_CURR_K1, false);
		GPIO_SetState(GPIO_V_CURR_K2, true);
	}
	else
	{
		GPIO_SetState(GPIO_V_CURR_K1, true);
		GPIO_SetState(GPIO_V_CURR_K2, false);
	}
}
//-----------------------------

void LL_V_IlimLowRange()
{
	GPIO_SetState(GPIO_V_LOW_CURRENT, false);
	GPIO_SetState(GPIO_V_HIGH_CURRENT, true);
}
//-----------------------------

void LL_V_IlimHighRange()
{
	GPIO_SetState(GPIO_V_HIGH_CURRENT, false);
	GPIO_SetState(GPIO_V_LOW_CURRENT, true);
}
//-----------------------------

void LL_V_Diagnostic(bool State)
{
	GPIO_SetState(GPIO_V_DIAG_CTRL, !State);
}
//-----------------------------

// C source
void LL_I_Start(bool State)
{
	GPIO_SetState(GPIO_C_C_START, State);
}
//-----------------------------

void LL_I_Enable(bool State)
{
	GPIO_SetState(GPIO_C_ENABLE, State);
}
//-----------------------------

void LL_I_SetDAC(Int16U Data)
{
	DAC_SetValueCh1(DAC1, Data);
}
//-----------------------------

void LL_I_Diagnostic(bool State)
{
	GPIO_SetState(GPIO_C_DIAG_CTRL, State);
}
//-----------------------------

void LL_ExtDACSendData(Int16U Data)
{
	GPIO_SetState(GPIO_C_EXT_DAC_CS, false);
	SPI_WriteByte(SPI1, Data);
	GPIO_SetState(GPIO_C_EXT_DAC_CS, true);

	GPIO_SetState(GPIO_C_EXT_DAC_LDAC, false);
	DELAY_US(1);
	GPIO_SetState(GPIO_C_EXT_DAC_LDAC, true);
}
//-----------------------------

void LL_ExDACVCutoff(Int16U Value)
{
	LL_ExtDACSendData(Value);
}
//-----------------------------

void LL_ExDACVNegative(Int16U Value)
{
	LL_ExtDACSendData(Value | EXT_DAC_B);
}
//-----------------------------
