
#ifndef  SPI_H
#define  SPI_H

#include "stdint.h"
#include "LPC1100.h"

// このデバイスがマスター
#define    SPI0_MASTER
// CS はGPIOで制御するかどうか
#define    SPI0_USE_CS_GPIO

#ifdef    SPI0_USE_CS_GPIO
  #ifdef    SPI0_MASTER
    #define CS_Low()  { GPIO0DATA &= ~_BV(2); }
    #define CS_High() { GPIO0DATA |= _BV(2); }
  #else
    #define CS_Low()
    #define CS_High()
  #endif
#endif



int Init_SPI0( uint8_t bitlen );

uint16_t SPI0_TxRx( uint16_t txdata );
int SPI0_Send( const uint16_t* data, uint32_t datalen );
int SPI0_Read( uint16_t* dst, uint32_t dstlen );

#endif
