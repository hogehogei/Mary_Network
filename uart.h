
#ifndef  UART_H
#define  UART_H

#include "stdint.h"

int Init_UART(void);
int UART_Recv( uint8_t* rx, uint32_t len );
int UART_Send( const uint8_t* tx, uint32_t len );
void UART_ClearRecvBuffer(void);

int UART_IsPresentRecvData(void);
void UART_Print( const char* str );
void UART_NewLine(void);
void UART_HexPrint( const uint8_t* hex, int len );

// 割り込みハンドラに登録すること！
void UART_IRQHandler(void);

#endif
