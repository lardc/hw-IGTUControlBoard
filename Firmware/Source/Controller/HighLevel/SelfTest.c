// Header
//
#include "SelfTest.h"
#include "Calibration.h"
#include "Controller.h"
#include "LowLevel.h"
#include "DataTable.h"
#include "Math.h"
#include "Qg.h"
#include "Delay.h"
#include "TOCUHP.h"

// Definitions
//
#define ST_V_VOLTAGE_LOW			10.0f
#define ST_V_VOLTAGE_HIGH			30.0f

#define ST_V_LOAD_RESISTANCE		150
#define ST_V_RINT_I_RANGE0			8600
#define ST_V_RINT_I_RANGE1			34
#define ST_V_RINT_I_RANGE2			34
//
#define ST_I_CURRENT_STAGE0			20
#define ST_I_CURRENT_STAGE1			300
#define ST_I_PULSE_DURATION			100
#define ST_I_V_SET_STAGE0			5
#define ST_I_V_SET_STAGE1			20

#define ST_I_Q_REF_STAGE0			1150
#define ST_I_Q_REF_STAGE1			4200
//

// Functions
//
void ST_Process()
{
	float ErrorV, ErrorI, ErrorQ, Voltage;
	static Int16U PreviousStage = 0;

	switch(CONTROL_SubState)
	{
		case SS_V_Prepare_Voltage:
			PreviousStage = SS_V_Prepare_Voltage;
			DataTable[REG_CAL_I] = ST_V_VOLTAGE_LOW / ST_V_LOAD_RESISTANCE * 1000;
			DataTable[REG_CAL_VP] = DataTable[REG_CAL_I] * (ST_V_RINT_I_RANGE2 + ST_V_LOAD_RESISTANCE) / 1000;

			LL_V_Diagnostic(true);
			DELAY_MS(20);

			CAL_V_Prepare();
			break;

		case SS_V_Prepare_Irange0:
			PreviousStage = SS_V_Prepare_Irange0;
			DataTable[REG_CAL_I] = V_I_R0_MAX;
			DataTable[REG_CAL_VP] = ST_V_VOLTAGE_HIGH;

			LL_V_Diagnostic(true);
			DELAY_MS(20);

			CAL_V_Prepare();
			break;

		case SS_V_Prepare_Irange1:
			PreviousStage = SS_V_Prepare_Irange1;
			DataTable[REG_CAL_I] = V_I_R1_MAX;
			DataTable[REG_CAL_VP] = V_I_R1_MAX * (ST_V_RINT_I_RANGE1 + ST_V_LOAD_RESISTANCE) / 1000;

			LL_V_Diagnostic(true);
			DELAY_MS(20);

			CAL_V_Prepare();
			break;

		case SS_V_Prepare_Irange2:
			PreviousStage = SS_V_Prepare_Irange2;
			DataTable[REG_CAL_I] = ST_V_VOLTAGE_HIGH / (ST_V_RINT_I_RANGE1 + ST_V_LOAD_RESISTANCE) * 1000;
			DataTable[REG_CAL_VP] = ST_V_VOLTAGE_HIGH;

			LL_V_Diagnostic(true);
			LL_I_Diagnostic(false);
			DELAY_MS(20);

			CAL_V_Prepare();
			break;

		case SS_V_Check:
			Voltage = DataTable[REG_CAL_I_RESULT] * ST_V_LOAD_RESISTANCE / 1000;

			if(Voltage > VGS_VOLTAGE_MIN && Voltage < VGS_VOLTAGE_MAX)
				ErrorV = fabsf((DataTable[REG_CAL_V_RESULT] - Voltage) / Voltage * 100);
			else
				ErrorV = 0;

			ErrorI = fabsf((DataTable[REG_CAL_I_RESULT] - DataTable[REG_CAL_I]) / DataTable[REG_CAL_I] * 100);

			if(ErrorV <= DataTable[REG_ST_CHECK_ERROR] && ErrorI <= DataTable[REG_ST_CHECK_ERROR])
			{
				switch(PreviousStage)
				{
					case SS_V_Prepare_Voltage:
						CONTROL_SetDeviceState(DS_SelfTest, SS_V_Prepare_Irange0);
						break;

					case SS_V_Prepare_Irange0:
						CONTROL_SetDeviceState(DS_SelfTest, SS_V_Prepare_Irange1);
						break;

					case SS_V_Prepare_Irange1:
						CONTROL_SetDeviceState(DS_SelfTest, SS_V_Prepare_Irange2);
						break;

					case SS_V_Prepare_Irange2:
						CONTROL_SetDeviceState(DS_SelfTest, SS_I_PrepareStage0);
						break;

					default:
						break;
				}
			}
			else
			{
				DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_ST_V_SOURCE_ERROR);
			}
			break;

		case SS_I_PrepareStage0:
			PreviousStage = SS_I_PrepareStage0;

			LL_I_Diagnostic(true);
			TOCUHP_EmulatedState(true);

			DataTable[REG_QG_I_DURATION] = ST_I_PULSE_DURATION;
			DataTable[REG_QG_V_CUTOFF] = ST_I_V_SET_STAGE0;
			DataTable[REG_QG_V_NEGATIVE] = ST_I_V_SET_STAGE0;
			DataTable[REG_QG_I] = ST_I_CURRENT_STAGE0;

			QG_Prepare();
			break;

		case SS_I_PrepareStage1:
			PreviousStage = SS_I_PrepareStage1;

			LL_I_Diagnostic(true);

			DataTable[REG_QG_I_DURATION] = ST_I_PULSE_DURATION;
			DataTable[REG_QG_V_CUTOFF] = ST_I_V_SET_STAGE1;
			DataTable[REG_QG_V_NEGATIVE] = ST_I_V_SET_STAGE1;
			DataTable[REG_QG_I] = ST_I_CURRENT_STAGE1;

			QG_Prepare();
			break;

		case SS_I_Check:
			if(PreviousStage == SS_I_PrepareStage0)
				ErrorQ = fabsf((DataTable[REG_QG_RESULT] - ST_I_Q_REF_STAGE0) / ST_I_Q_REF_STAGE0 * 100);
			else
				ErrorQ = fabsf((DataTable[REG_QG_RESULT] - ST_I_Q_REF_STAGE1) / ST_I_Q_REF_STAGE1 * 100);

			if(ErrorQ <= DataTable[REG_ST_CHECK_ERROR])
			{
				if(PreviousStage == SS_I_PrepareStage0)
					CONTROL_SetDeviceState(DS_SelfTest, SS_I_PrepareStage1);
				else
				{
					TOCUHP_EmulatedState(false);
					DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_OK;
					CONTROL_SetDeviceState(DS_Ready, SS_None);
				}
			}
			else
			{
				PreviousStage = 0;
				TOCUHP_EmulatedState(false);
				DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
				CONTROL_SwitchToFault(DF_ST_I_SOURCE_ERROR);
			}
			break;

		default:
			break;
	}
}
//--------------------------

