#ifndef CONTROLLER_HIGHLEVEL_IGES_H_
#define CONTROLLER_HIGHLEVEL_IGES_H_

// Include
//
#include "Board.h"
#include "stdinc.h"

// Variables
//
extern bool PAU_SyncFlag;

// Functions
//
void IGES_Prepare();
void IGES_Process();
void IGES_SaveResults();

#endif /* CONTROLLER_HIGHLEVEL_IGES_H_ */
