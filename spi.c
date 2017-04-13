
#include "spi.h"
#include "LPC1100.h"
#include "systick.h"
#include "uart.h"

int Init_SPI0( uint8_t bitlen )
{
	// 1回でやり取りするデータ長
	// 4bit から 16bit まで設定可能
	if( bitlen < 4 || bitlen > 16 ){
		return 0;  
	}

	// SPI0へのクロック供給を有効
	SYSAHBCLKCTRL |= (1 << 11);

	// SCK0 を PIO0_6  に割り当て
	IOCON_SCK0_LOC = 0x2;
	// SPI0のピン設定
	IOCON_PIO0_6 = 0x02;    // SCK0
	IOCON_PIO0_8 = 0x01;    // MISO0
	IOCON_PIO0_9 = 0x01;    // MOSI0
#ifdef    SPI0_USE_CS_GPIO
	GPIO0DIR |= _BV(2);     // SSEL0 chipselect by GPIO
	CS_High();
#else
	IOCON_PIO0_2 = 0x01;  // SSEL0 chipselect
#endif

	// SPI0をリセット
	PRESETCTRL &= ~0x01;
	PRESETCTRL |= 0x01;
	// SPI0のシステムクロック 36Mhz
	SSP0CLKDIV = 1;
	// SPI0の bit frequency 9Mhz
	SSP0CPSR = 2;

	// 1回でやり取りするデータ長
	// mode 0, 0
#ifdef    SPI0_MASTER
	SSP0CR0 = (0x0F & (bitlen-1));  // CPOL=0, CPHA=0
#else
	SSP0CR0 = (0x0F & (bitlen-1));  // CPOL=0, CPHA=0
#endif

	// SSP0有効化　ここまでにレジスタの設定すませる
#ifdef    SPI0_MASTER
	//SSP0CR1 |= 0x01;    // loopback
	SSP0CR1 |= 0x02;    // master mode
#else
	SSP0CR1 |= 0x04;    // slave mode
	SSP0CR1 |= 0x02;
#endif

	return 1;
}

uint16_t SPI0_TxRx( uint16_t txdata )
{
	uint16_t rxdata;

	// Send
	// TxFIFO Full / Busy の間は待つ
#ifdef    SPI0_MASTER
	while( !(SSP0SR & 0x02) || (SSP0SR & 0x10) );
	SSP0DR = txdata;
	// SPIがデータを送り終わるまで待つ
	while( SSP0SR & 0x10 );
#endif

	// Recv
	// データが来るまで待つ
	while( !(SSP0SR & 0x04 ) ) ;
	rxdata = SSP0DR;

	return rxdata;
}

int SPI0_Send( const uint16_t* data, uint32_t datalen )
{
	int i = 0;
	for( i = 0; i < datalen; ++i ){
		SPI0_TxRx( data[i] );
	}

	// 送信したバイト数を返す
	return i;
}

int SPI0_Read( uint16_t* dst, uint32_t dstlen )
{
	int i = 0;
	for( i = 0; i < dstlen; ++i ){
		dst[i] = SPI0_TxRx(0x00);
	}

	return i;
}



