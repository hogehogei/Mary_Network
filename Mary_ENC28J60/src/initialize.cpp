/*
 * initialize.cpp
 *
 *  Created on: 2019/03/10
 *      Author: hogehogei
 */

#include "LPC1100.h"
#include "led.h"
#include "timer32.h"
#include "uart.h"
#include "systick.h"

// For initialize static constructor
extern void (*__init_array_start[]) ();
extern void (*__init_array_end[]) ();

void Initialize_CPU()
{
	/* Configure BOD control (Reset on Vcc dips below 2.7V) */
	BODCTRL = 0x13;

	/* Configure system clock generator (36MHz system clock with IRC) */
	MAINCLKSEL = 0;							/* Select IRC as main clock */
	MAINCLKUEN = 0; MAINCLKUEN = 1;
	FLASHCFG = (FLASHCFG & 0xFFFFFFFC) | 2;	/* Set wait state for flash memory (1WS) */
#if 1
	SYSPLLCLKSEL = 0;						/* Select IRC for PLL-in */
	SYSPLLCLKUEN = 0; SYSPLLCLKUEN = 1;
	SYSPLLCTRL = (3 - 1) | (2 << 6);		/* Set PLL parameters (M=3, P=4) */
	PDRUNCFG &= ~0x80;						/* Enable PLL */
	while ((SYSPLLSTAT & 1) == 0) ;			/* Wait for PLL locked */
#endif
	SYSAHBCLKDIV = 1;						/* Set system clock divisor (1) */
	MAINCLKSEL = 3;							/* Select PLL-out as main clock */
	MAINCLKUEN = 0; MAINCLKUEN = 1;

	SYSAHBCLKCTRL |= 0x1005F;
}

void Initialize_Peripheral()
{
	// UARTの初期化
	Init_UART();
	UART_Print( "Init_UART()" );
	// オンボードLEDの初期化
	Init_LED();
	UART_Print( "Init_LED()" );
	// Systick Timerの初期化
	Init_Systick();
	UART_Print( "Init_Systick()");
	// Timer32B1 の初期化 パケット送信タイマーに用いる
	Init_Timer32B1( 1000 );    // 1ms
	UART_Print( "Init_Timer32B1()" );
}

__attribute__ ((section(".after_vectors")))
void Call_Static_Initializers()
{
	int count = __init_array_end - __init_array_start;
	for( int i = 0; i < count; ++i ){
		__init_array_start[i]();
	}
}

