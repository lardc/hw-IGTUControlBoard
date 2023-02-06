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
	GPIO_SetState(GPIO_INDICATION, State);
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

bool LL_FeedbackPAU()
{
	return GPIO_GetState(GPIO_FB_PAU);
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

void LL_V_CoefCSensLowRange() {
	GPIO_SetState(GPIO_V_CURR_K1, false);
	GPIO_SetState(GPIO_V_CURR_K2, true);
}
//-----------------------------

void LL_V_CoefCSensHighRange() {
	GPIO_SetState(GPIO_V_CURR_K2, false);
	GPIO_SetState(GPIO_V_CURR_K1, true);
}
//-----------------------------

void LL_V_CLimitLowRange() {
	GPIO_SetState(GPIO_V_LOW_CURRENT, false);
	GPIO_SetState(GPIO_V_HIGH_CURRENT, true);
}
//-----------------------------

void LL_V_CLimitHighRange() {
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
void LL_C_CStart(bool State)
{
	GPIO_SetState(GPIO_C_C_START, !State);
}
//-----------------------------

void LL_C_CEnable(bool State)
{
	GPIO_SetState(GPIO_C_ENABLE, State);
}
//-----------------------------

void LL_C_CSetDAC(Int16U Data)
{
	DAC_SetValueCh1(DAC1, Data);
}
//-----------------------------

void LL_C_Diagnostic(bool State)
{
	GPIO_SetState(GPIO_C_DIAG_CTRL, State);
}
//-----------------------------

void LL_ExtDACSync(bool State)
{
	GPIO_SetState(GPIO_C_EXT_DAC_CS, State);
}
//-----------------------------

void LL_ExtDACLDAC(bool State)
{
	GPIO_SetState(GPIO_C_EXT_DAC_LDAC, State);
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

void LL_ExDACVCutoff(float Value)
{
	Int16U Data = CU_C_VCutoffToExtDAC(Value);
	LL_ExtDACSendData(Data);
}
//-----------------------------

void LL_ExDACVNegative(float Value)
{
	Int16U Data = CU_C_VNegativeToExtDAC(Value);
	Data |= EXT_DAC_B;
	LL_ExtDACSendData(Data);
}
//-----------------------------
