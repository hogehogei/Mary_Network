/*
 * interrupt.c
 *
 *  Created on: 2018/11/12
 *      Author: hogehogei
 */

#include <stdint.h>
#include "LPC1100.h"
#include "interrupt.h"
#include "uart.h"
#include "global.hpp"

__attribute__ ((section(".after_vectors")))
void CT32B1_IRQHandler(void)
{
	if( TMR32B1IR & 0x01 ){
		if( gTimer32B1_Callback ){
			(*gTimer32B1_Callback)();
		}
		TMR32B1IR = (1 << 0);
	}
}

__attribute__ ((section(".after_vectors")))
void UART_IRQHandler(void)
{
	while(1){
		uint32_t int_flag = U0IIR;
		// 割り込みが残ってないなら抜ける
		if( (int_flag & 0x01) ){
			break;
		}

		uint32_t t = 0;
		switch( int_flag & 0x0E ){
		case 2:    // THER
			// 送信バッファに残っているデータをすべて送る
			while( UART_SendByte() ) ;
			break;
		case 4:   // Recieve Data Available
			UART_MoveRxFIFODataToRxBuf();
			break;
		default:  // Rx FIFO Error
			// エラーをクリア
			t = U0LSR;
			// U0RBR の中身を読み捨てる
			t = U0RBR;
			break;
		}
	}
}

__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void)
{
	if( gSystick_WaitCalled ){
		++gSystick_WaitCnt;
	}
}

