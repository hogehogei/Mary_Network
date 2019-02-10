
#include <drv/spi.h>
#include "LPC1100.h"
#include "systick.h"
#include "uart.h"


SPI SPI_Drv::m_SPI[k_SPI_Channels];

bool SPI_Drv::Initialize( const SPI_Drv::Settings& settings )
{
	// channel 0 しか対応しない
	if( settings.channel > 0 ){
		return false;
	}
	// 1回でやり取りするデータ長
	// 4bit から 16bit まで設定可能
	if( settings.bitlen < 4 || settings.bitlen > 16 ){
		return false;
	}
	// SLAVE には対応しない
	if( !(settings.role == ROLE_MASTER) ){
		return false;
	}

	// SPI0へのクロック供給を有効
	__enable_ahbclk( SYSAHBCLK_SPI );

	// SCK0 を PIO0_6  に割り当て
	IOCON_SCK0_LOC = 0x2;
	// SPI0のピン設定
	IOCON_PIO0_6 = 0x02;    // SCK0
	IOCON_PIO0_8 = 0x01;    // MISO0
	IOCON_PIO0_9 = 0x01;    // MOSI0

	// SPI0をリセット
	PRESETCTRL &= ~0x01;
	PRESETCTRL |= 0x01;
	// SPI0のシステムクロック 36Mhz
	SSP0CLKDIV = 1;
	// SPI0の bit frequency 18Mhz
	SSP0CPSR = 2;

	// 1回でやり取りするデータ長
	// mode 0, 0
	SSP0CR0 = (0x0F & (settings.bitlen-1));  // CPOL=0, CPHA=0

	// SSP0有効化　ここまでにレジスタの設定すませる
	//SSP0CR1 |= 0x01;		// loopback
	SSP0CR1 |= 0x02;		// master mode
	//SSP0CR1 |= 0x04;		// slave mode

	return true;
}

SPI& SPI_Drv::Instance( uint8_t channel )
{
	return m_SPI[0];
}

SPI::SPI()
	: m_Is_Initialized( false )
{}

SPI::SPI( bool is_initialized )
	: m_Is_Initialized( is_initialized )
{}

SPI::~SPI()
{}

uint16_t SPI::TxRx( uint16_t txdata )
{
	uint16_t rxdata;

	// Send
	// TxFIFO Full / Busy の間は待つ
	while( !(SSP0SR & 0x02) || (SSP0SR & 0x10) );
	SSP0DR = txdata;
	// SPIがデータを送り終わるまで待つ
	while( SSP0SR & 0x10 );

	// Recv
	// データが来るまで待つ
	while( !(SSP0SR & 0x04 ) ) ;
	rxdata = SSP0DR;

	return rxdata;
}

uint32_t SPI::Send_U16( const uint16_t* data, uint32_t datalen )
{
	uint32_t i = 0;
	for( i = 0; i < datalen; ++i ){
		TxRx( data[i] );
	}

	// 送信したバイト数を返す
	return i;
}

uint32_t SPI::Send_U8( const uint8_t* data, uint32_t datalen )
{
	uint32_t i = 0;
	for( i = 0; i < datalen; ++i ){
		TxRx( data[i] );
	}

	// 送信したバイト数を返す
	return i;
}

uint32_t SPI::Recv_U16( uint16_t* dst, uint32_t dstlen )
{
	uint32_t i = 0;
	for( i = 0; i < dstlen; ++i ){
		dst[i] = TxRx(0x00);
	}

	return i;
}
