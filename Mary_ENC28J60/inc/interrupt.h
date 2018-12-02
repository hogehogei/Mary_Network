/*
 * interrupt.h
 *
 *  Created on: 2018/11/12
 *      Author: hogehogei
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#if defined (__cplusplus)
extern "C" {
#endif

void UART_IRQHandler(void);
void CT32B1_IRQHandler(void);
void SysTick_Handler(void);
void RecvPacket(void);

#if defined (__cplusplus)
} // extern "C"
#endif

#endif /* INTERRUPT_H_ */
