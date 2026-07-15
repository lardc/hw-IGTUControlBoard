// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] =
{
	{"REG_IT_SETPOINT",	 			DT_Float,	1},
	{"REG_UG_SETPOINT", 			DT_Float,	1},

	{"REG_DEV_STATE", 				DT_Float,	1},
	{"REG_FAULT_REASON", 			DT_Float,	1},
	{"REG_DISABLE_REASON", 			DT_Float,	1},
	{"REG_WARNING", 				DT_Float,	1},
	{"REG_PROBLEM", 				DT_Float,	1},
	{"REG_OP_RESULT", 				DT_Float,	1},
	{"REG_SELF_TEST_OP_RESULT", 	DT_Float,	1},
	{"REG_SUB_STATE",			 	DT_Float,	1},

	{"It Digital",					DT_Float,	VALUES_x_SIZE},
	{"Ut Digital",					DT_Float,	VALUES_x_SIZE},
	{"Ug Digital",					DT_Float,	VALUES_x_SIZE},
	{"Ug Error",					DT_Float,	VALUES_x_SIZE},
	{"Ig Digital",					DT_Float,	VALUES_x_SIZE},

	{"Power Values Counter",		DT_Int16U,	1},
	{"Gate Values Counter",			DT_Int16U,	1}

};
Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);
