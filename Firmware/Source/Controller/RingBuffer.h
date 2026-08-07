#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

// Include
#include "stdinc.h"
#include "SysConfig.h"

// Defines
#define RINGBUF_MAX_AVG_WINDOW_US		20000
#define RINGBUF_MAX_SIZE				((Int16U)(RINGBUF_MAX_AVG_WINDOW_US / (Int32U)TIMER15_uS))

// Functions
//
void RINGBUF_Reset(Int16U Size);
void RINGBUF_AddSample(float I, float U);
float RINGBUF_GetAvgI();
float RINGBUF_GetAvgU();
Boolean RINGBUF_IsFull();

#endif // __RINGBUFFER_H
