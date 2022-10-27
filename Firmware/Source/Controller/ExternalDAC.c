// Header
#include "ExternalDAC.h"
// Include
#include "Board.h"
#include "LowLevel.h"
#include "ConvertUtils.h"
#include "Global.h"

// Functions
//

void ExDAC_IUCutoff(float Value)
{
	Int16U Data = CU_UCutoffToExtDAC(Value);
	LL_ExtDACSendData(Data);
}
//-----------------------------

void ExDAC_IUNegative(float Value)
{
	Int16U Data = CU_UNegativeToExtDAC(Value);
	Data |= EXT_DAC_B;
	LL_ExtDACSendData(Data);
}
//-----------------------------
