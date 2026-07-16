// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] =
{
	{"REG_VGS_I_TRIG",	 			DT_Float,	1},
	{"REG_VGS_V_MAX", 				DT_Float,	1},

	{"REG_QG_V_CUTOFF",	 			DT_Float,	1},
	{"REG_QG_V_NEGATIVE", 			DT_Float,	1},
	{"REG_QG_I",	 				DT_Float,	1},
	{"REG_QG_I_DURATION", 			DT_Float,	1},
	{"REG_QG_I_POWER",	 			DT_Float,	1},
	{"REG_QG_V_POWER",	 			DT_Float,	1},

	{"REG_IGES_V",	 				DT_Float,	1},
	{"REG_IGES_RANGE",	 			DT_Float,	1},

	{"REG_SERTIFICATION",	 		DT_Float,	1},
	{"REG_CAL_VP",	 				DT_Float,	1},
	{"REG_CAL_VN",	 				DT_Float,	1},
	{"REG_CAL_I",	 				DT_Float,	1},

	{"REG_DEV_STATE", 				DT_Float,	1},
	{"REG_FAULT_REASON", 			DT_Float,	1},
	{"REG_DISABLE_REASON", 			DT_Float,	1},
	{"REG_WARNING", 				DT_Float,	1},
	{"REG_PROBLEM", 				DT_Float,	1},
	{"REG_OP_RESULT", 				DT_Float,	1},
	{"REG_SELF_TEST_OP_RESULT", 	DT_Float,	1},
	{"REG_SUB_STATE",			 	DT_Float,	1},

	{"REG_VGS_RESULT", 				DT_Float,	1},
	{"REG_VGS_I_RESULT", 			DT_Float,	1},
	{"REG_QG_RESULT", 				DT_Float,	1},
	{"REG_QG_I_RESULT", 			DT_Float,	1},
	{"REG_IGES_RESULT", 			DT_Float,	1},
	{"REG_RES_RESULT", 				DT_Float,	1},

	{"REG_CAL_V_RESULT", 			DT_Float,	1},
	{"REG_CAL_VN_RESULT",			DT_Float,	1},
	{"REG_CAL_I_RESULT",			DT_Float,	1},

	{"REG_EXT_UNIT_ERROR_CODE", 	DT_Float,	1},
	{"REG_EXT_UNIT_FUNCTION",		DT_Float,	1},
	{"REG_EXT_UNIT_EXT_DATA",		DT_Float,	1},

	{"Voltage values",				DT_Float,	VALUES_x_SIZE},
	{"Current values",				DT_Float,	VALUES_x_SIZE},
	{"Regulator error",				DT_Float,	VALUES_x_SIZE},
	{"Regulator out",				DT_Float,	VALUES_x_SIZE},

	{"Power Values Counter",		DT_Int16U,	1},
	{"Regulator Values Counter",	DT_Int16U,	1}
};
Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);

CounterDescription CounterStorageDescription[] = {0};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
