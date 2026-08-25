// Header
#include "StorageDescription.h"

#include "Global.h"

RecordDescription StorageDescription[] =
{
	{"REG_DEV_STATE",			DT_Float, 1},
	{"REG_FAULT_REASON",		DT_Float, 1},
	{"REG_DISABLE_REASON",		DT_Float, 1},
	{"REG_WARNING",				DT_Float, 1},
	{"REG_PROBLEM",				DT_Float, 1},
	{"REG_OP_RESULT",			DT_Float, 1},
	{"REG_DEV_SUBSTATE",		DT_Float, 1},

	{"REG_UGE_TH",				DT_Float, 1},
	{"REG_IGES_RESULT",			DT_Float, 1},
	{"REG_THERM_RESIS",			DT_Float, 1},

	{"REG_DIAG_CURRENT",		DT_Float, 1},
	{"REG_DIAG_VOLTAGE",		DT_Float, 1},
	{"REG_DIAG_POT_VOLTAGE",	DT_Float, 1},
	{"REG_EP_DATA_STEP",		DT_Float, 1},

	{"EP_RegulatorIg",			DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_RegulatorUg",			DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_RegulatorUpot",		DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_RegulatorSetpoint",	DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_RegulatorCorrection",	DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_RegulatorError",		DT_Float, VALUES_DEBUG_RGLTR_SIZE},
	{"EP_DACRaw",				DT_Float, VALUES_DEBUG_RGLTR_SIZE},
};

Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);

const CounterDescription CounterStorageDescription[] =
{
	{"0. Current Channel 0"},
	{"1. Current Channel 1"},
	{"2. Current Channel 2"},
	{"3. Current Channel 3"},
	{"4. Current Channel 4"},
	{"5. Current Channel 5"},
	{"6. Current Channel 6"},
	{"7. Current Channel 7"},
	{"8. Potential entry relay"},
	{"9. Self test relay"},
	{"10. Test load relay"},
	{"11. Polarity switch"},
};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
