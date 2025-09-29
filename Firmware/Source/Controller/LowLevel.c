// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "math.h"

typedef enum __PulseState
{
	PS_None,
	PS_Start,
	PS_BaseShift,
	PS_Rise,
	PS_Plate,
	PS_Sine,
	PS_Stop
} PulseState;

static PulseState State;

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
	State = PS_Start;
	TIM_Start(TIM3);
}
//-----------------------------

void LL_HandlePulse()
{
	static Int16U Counter = 0, PulseAmplitude = 0, MaxPulse = 0;
	float SineMul = 0;

	switch(State)
	{
		case PS_Start:
			State = PS_BaseShift;
			Counter = 0;
			PulseAmplitude = DataTable[REG_DBG_BASE_SHIFT];
			MaxPulse = DataTable[REG_DBG_BASE_SHIFT] + DataTable[REG_DBG_PULSE_AMPL];
			SineMul = M_PI / DataTable[REG_DBG_PULSE_TIME];
			LL_SetDAC(PulseAmplitude);
			break;

		case PS_BaseShift:
			Counter += TIMER3_uS;
			if(Counter >= DataTable[REG_DBG_BASE_TIME])
			{
				Counter = 0;
				State = DataTable[REG_DBG_IS_SINE] ? PS_Sine : PS_Rise;
			}
			break;

		case PS_Rise:
			PulseAmplitude += DataTable[REG_DBG_PULSE_STEP];
			if(PulseAmplitude >= MaxPulse)
			{
				PulseAmplitude = MaxPulse;
				State = PS_Plate;
			}
			LL_SetDAC(PulseAmplitude);
			break;

		case PS_Sine:
			{
				Int16U SineVal = DataTable[REG_DBG_PULSE_AMPL] * sinf(M_PI * Counter / DataTable[REG_DBG_PULSE_TIME]);
				LL_SetDAC(DataTable[REG_DBG_BASE_SHIFT] + SineVal);
			}
		case PS_Plate:
			Counter += TIMER3_uS;
			if(Counter >= DataTable[REG_DBG_PULSE_TIME])
				State = PS_Stop;
			break;

		case PS_Stop:
			State = PS_None;
			PulseAmplitude = 0;
			TIM_Stop(TIM3);
			LL_SetDAC(0);
			break;

		default:
			break;
	}
}
//-----------------------------
