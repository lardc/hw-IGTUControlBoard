// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"

Int16U PlateCounter, PulseAmplitude;
bool PlateReady;

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_SetDAC(Int16U Value)
{
	GPIO_SetState(GPIO_SPI_SS, false);
	SPI_WriteByte(SPI1, Value);
	GPIO_SetState(GPIO_SPI_SS, true);
	GPIO_SetState(GPIO_SPI_LDAC, false);
	DELAY_US(1);
	GPIO_SetState(GPIO_SPI_LDAC, true);
	DELAY_US(1);
}
//-----------------------------

void LL_StartPulse()
{
	PlateReady = false;
	PulseAmplitude = 0;
	PlateCounter = 0;

	TIM_Start(TIM3);
}
//-----------------------------

void LL_HandlePulse()
{
	if(PlateReady)
	{
		PlateCounter += TIMER3_uS;
		if(PlateCounter >= DataTable[REG_DBG_PULSE_TIME])
		{
			PulseAmplitude = 0;
			TIM_Stop(TIM3);
		}
	}
	else
	{
		PulseAmplitude += DataTable[REG_DBG_PULSE_STEP];
		if(PulseAmplitude >= DataTable[REG_DBG_PULSE_AMPL])
		{
			PulseAmplitude = DataTable[REG_DBG_PULSE_AMPL];
			PlateReady = true;
		}
	}
	LL_SetDAC(PulseAmplitude);
}
//-----------------------------
