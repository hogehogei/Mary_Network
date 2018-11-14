
#ifndef TIMER32_H
#define TIMER32_H

#include "stdint.h"
#include "LPC1100.h"

int Init_Timer32B1( uint32_t irq_micro_sec );
int Timer32B1_SetCallback( void (*callback_func)(void) );


#endif
