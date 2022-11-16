// Header
#include "ExternalDAC.h"
// Include
#include "Board.h"
#include "LowLevel.h"
#include "ConvertUtils.h"
#include "Global.h"

// Functions
//

void ExDAC_C_VCutoff(float Value)
{
	Int16U Data = CU_C_VCutoffToExtDAC(Value);
	LL_ExtDACSendData(Data);
}
//-----------------------------

void ExDAC_C_VNegative(float Value)
{
	Int16U Data = CU_C_VNegativeToExtDAC(Value);
	Data |= EXT_DAC_B;
	LL_ExtDACSendData(Data);
}
//-----------------------------
