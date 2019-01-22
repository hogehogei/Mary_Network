#include "LPC1100.h"
#include "drv/spi.h"
#include "drv/gpio.hpp"
#include "ENC28J60.h"
#include "systick.h"
#include "uart.h"
#include "network.h"


ENC28J60::ENC28J60()
{}

ENC28J60::~ENC28J60()
{}

bool ENC28J60::Initialize( const Eth_Settings& settings, uint8_t spi_ch, const GPIO& cs )
{
	m_Settings	= settings;
	m_SPI		= SPI_Drv::Instance( spi_ch );
	m_CS		= cs;
	m_CurrentBank = 0;

	// Mary側の割り込み PIO0_3 を割り込み判定に使用
	// High->Low falling edge 時に検出
	GPIO0IS  &= ~_BV(3);
	GPIO0IBE &= ~_BV(3);
	GPIO0IEV &= ~_BV(3);
	GPIO0IE  |= _BV(3);
	__enable_irqn( PIO_0_IRQn );

	// チップセレクトをHiに固定
	m_CS.Hi();

	// システムリセット
	Reset();
	// PHYの初期化
	Init_PHY();
	// Pkt Buffer　の初期化
	Init_PktBuffer();
	// Pkt Filter の初期化
	Init_PktFilter();
	// MACの初期化
	Init_MAC();
	// 割り込み設定
	Init_Interrupt();
	// Ethernet Control 初期化
	Init_EthControl();

	// デバッグ用
	Show_Setting();

	return true;
}

// Utility Functions
void ENC28J60::BitFieldSet( uint8_t reg, uint8_t val )
{
	__disable_irq();
	m_CS.Lo();
	m_SPI.TxRx( _BFS(reg) );
	m_SPI.TxRx( val );
	m_CS.Hi();
	__enable_irq();
}

void ENC28J60::BitFieldClear( uint8_t reg, uint8_t val )
{
	__disable_irq();
	m_CS.Lo();
	m_SPI.TxRx( _BFC(reg) );
	m_SPI.TxRx( val );
	m_CS.Hi();
	__enable_irq();
}

void ENC28J60::SwitchBank( uint8_t reg )
{
	uint8_t bank = GETBANK( reg );
	if( bank != m_CurrentBank ){
		BitFieldClear( ECON1, 0x03 );
		BitFieldSet( ECON1, bank );
		m_CurrentBank = bank;
	}
}

uint8_t ENC28J60::ReadCR( uint8_t reg )
{
	SwitchBank( reg );

	m_CS.Lo();
	uint16_t data;

	__disable_irq();
	m_SPI.TxRx( _RCR(reg) );
	data = m_SPI.TxRx(0x00);
	__enable_irq();

	if( IS_MAC_MII_REG(reg) ){
		// MAC/MII register の場合は 1バイト目はダミーで2バイト目が有効なデータ
		__disable_irq();
		data = m_SPI.TxRx(0x00);
		__enable_irq();

		volatile int i = 0;
		for( i = 0; i < MAC_MII_REGISTER_ACCESS_WAIT; ++i ){
			m_CS.Lo();
		}
	}
	m_CS.Hi();

	return data;
}

void ENC28J60::WriteCR( uint8_t reg, uint8_t data )
{
	SwitchBank( reg );

	m_CS.Lo();
	__disable_irq();
	m_SPI.TxRx( _WCR(reg) );
	m_SPI.TxRx( data );
	__enable_irq();

	if( IS_MAC_MII_REG(reg) ){
		volatile int i = 0;
		for( i = 0; i < MAC_MII_REGISTER_ACCESS_WAIT; ++i ){
			m_CS.Lo();
		}
	}
	m_CS.Hi();
}

void ENC28J60::ReadPHYReg( uint8_t phy_addr, uint8_t* reg_high, uint8_t* reg_low )
{
	WriteCR( MIREGADR, phy_addr );
	WriteCR( MICMD, 0x01 );    // MII Read Enable bit

	while( ReadCR(MISTAT) & 0x01 ) ; // BUSY の間待つ

	WriteCR( MICMD, 0x00 );    // Clear MII Read Enable bit

	*reg_low = ReadCR( MIRDL );
	*reg_high = ReadCR( MIRDH );
}

void ENC28J60::WritePHYReg( uint8_t phy_addr, uint8_t reg_high, uint8_t reg_low )
{
	WriteCR( MIREGADR, phy_addr );
	WriteCR( MIWRL, reg_low );
	WriteCR( MIWRH, reg_high );
}

void ENC28J60::ReadBufferMem( uint8_t* dst, uint8_t len )
{
	__disable_irq();
	m_CS.Lo();
	// Read buffer memory command
	m_SPI.TxRx( _RBM() );
	int i = 0;
	for( i = 0; i < len; ++i ){
		dst[i] = m_SPI.TxRx(0x00);
	}
	m_CS.Hi();
	__enable_irq();
}

void ENC28J60::WriteBufferMem( const uint8_t* data, uint16_t len )
{
	__disable_irq();
	m_CS.Lo();
	// Write buffer memory command
	m_SPI.TxRx( _WBM() );
	m_SPI.Send( reinterpret_cast<const uint16_t*>(data), len );
	m_CS.Hi();
	__enable_irq();
}


// Public functions
bool ENC28J60::Send( const PacketPtr& frame )
{
	// TXRTX:1 なら送信中なので待つ
	while( ReadCR(ECON1) & 0x08 ) {
		/*
		// TXERIF : 送信エラー発生ならリセット
		if( ReadCR(EIR) & 0x02 ){
			BitFieldSet( ECON1, 0x80 );
			BitFieldClear( ECON1, 0x80 );
		}
		*/
	}

	uint16_t tx_start_addr = TX_BUFFER_START;
	uint16_t tx_end_addr = (TX_BUFFER_START + frame->Size());

	// ENC28J60側の送信バッファ 書き込みスタートアドレス設定
	WriteCR( EWRPTL, tx_start_addr & 0xFF );
	WriteCR( EWRPTH, tx_start_addr >> 8 );
	// 書き込み終了アドレスも設定
	// ここに設定したアドレスが最終データになるようにする
	WriteCR( ETXNDL, tx_end_addr & 0xFF );
	WriteCR( ETXNDH, tx_end_addr >> 8 );

	// 書き込み
	// 書き込みデータの先頭にパケット送信コントロールデータ追加
	// macon3 の設定をそのまま使用
	uint8_t control_byte[1] = { 0x0E };
	WriteBufferMem( control_byte, 1 );
	WriteBufferMem( frame->Head(), frame->Size() );

	BitFieldSet( ECON1, 0x08 );    // TXRTS: Transmit Request to Send Enable

	return true;
}

bool ENC28J60::Recv( PacketPtr* frame )
{
	if( Get_RemainPacketCount() == 0 ){
		return false;
	}

	uint8_t header[6] = {0, };
	int status = 0;

	WriteCR( ERDPTL, m_Rx_NextPktPtr & 0xFF );
	WriteCR( ERDPTH, m_Rx_NextPktPtr >> 8 );

	ReadBufferMem( header, 6 );    // next packet pointer + status vector を読む
	m_Rx_NextPktPtr = header[0] | (header[1] << 8);
	uint16_t recv_byte = header[2] | (header[3] << 8);
	recv_byte -= 4;    // CRC は読まない

	// status vector から CRC Error チェック
	if( header[4] & (1 << 4) ){
		status = RECV_CRCERR;
	}

	PacketPtr rx_frame = Create_Packet( recv_byte );
	if( rx_frame.isNull() ){
		status = RECV_DROPPKT;
	}
	else {
		status = RECV_VALIDPKT;
		// 実際のデータを読む
		ReadBufferMem( rx_frame->Head(), rx_frame->Size() );
	}


	// 読み込みポインタを次のアドレスに進める
	// Rev.B4 Errata シート参照
	if( (m_Rx_NextPktPtr - 1) < RX_BUFFER_START || (m_Rx_NextPktPtr - 1) > RX_BUFFER_END ){
		WriteCR( ERXRDPTL, RX_BUFFER_END & 0xFF );
		WriteCR( ERXRDPTH, RX_BUFFER_END >> 8 );
	}
	else {
		WriteCR( ERXRDPTL, (m_Rx_NextPktPtr - 1) & 0xFF );
		WriteCR( ERXRDPTH, (m_Rx_NextPktPtr - 1) >> 8 );
	}

	// パケットを処理したので、パケットカウントを1つ減らす
	BitFieldSet( ECON2, (1 << 6) );

	*frame = rx_frame;

	return status == RECV_VALIDPKT ? true : false;
}

uint32_t ENC28J60::get_RxRemainPacketCount()
{
	return Get_RemainPacketCount();
}

bool ENC28J60::isLinkUp()
{
	uint8_t phstat2[2];
	ReadPHYReg( PHSTAT2, &phstat2[0], &phstat2[1] );

	// PHSTAT2 10bit LSTAT　をチェック
	// PHSTAT2 の Hi側を調べている
	return phstat2[0] & (1 << 2);
}

const uint8_t* ENC28J60::getMacAddr() const
{
	return m_Settings.macaddr;
}

uint8_t ENC28J60::Get_RemainPacketCount()
{
	// eratta sheet 参照
	// PKTIF だけでは受信パケットがあるかどうか判断できない時があるらしい。
	return ReadCR(EPKTCNT);
}

int ENC28J60::Interrupt_Callback()
{
	DisableInterrupt();
	int status = 0;
	uint8_t eir = ReadCR(EIR);

	if( eir & LINKIF ){
		status |= INT_LINKCHANGE;
		ReadCR( PHIR );
		// LINKIF はPHIRを読むと自動でクリアされる
	}
	if( eir & PKTIF ){
		status |= INT_RECVPKT;
	}
	if( eir & TXERIF ){
		status |= INT_TXERROR;
		// TXERIF: 送信エラー発生 Txリセット
		BitFieldSet( ECON1, (1 << 7) );
		BitFieldClear( ECON1, (1 << 7) );
		BitFieldClear( EIR, TXERIF );
	}
	if( eir & RXERIF ){
		status |= INT_RXERROR;
		// RXERIF: 受信エラー発生 Rxリセット
		BitFieldSet( ECON1, (1 << 6) );
		BitFieldClear( ECON1, (1 << 6) );
		// RXEN が自動でクリアされてしまうので再セット
		BitFieldSet( ECON1, (1 << 2) );
		BitFieldClear( EIR, RXERIF );
	}

	GPIO0IC |= _BV(3);

	// 割り込みを有効にする
	// パケット受信割り込みは、今送られてきたパケットをすべて処理したら割り込み許可する
	EnableTxRxErrorInterrupt();
	if( get_RxRemainPacketCount() == 0 ){
		// パケットを受信しつくしたので割り込み有効に
		EnableRecvPktInterrupt();
	}

	if( status & INT_LINKCHANGE ){
		UART_Print( "PHY link status change" );
	}
	if( status & INT_RXERROR ){
		UART_Print( "RxError, reset rx buffer" );
	}
	if( status & INT_TXERROR ){
		UART_Print( "TxError, reset tx buffer" );
	}

	return status;
}

void ENC28J60::Reset()
{
	m_SPI.TxRx( _SRC() );
	Systick_Wait( 1 );   // リセット後50us 以上待つ必要があるとのことなので、1ms待つ
	while( !(ReadCR(ESTAT) & 0x01) ) ;    // ClockReady になるまで待つ

	// PHYをリセット
	// PHY FullDuplex Mode
	UART_Print( "Setting Full Duplex Mode" );
	WritePHYReg( PHCON1, 0x81, 0x00 );
	UART_Print( "ENC28J60 reset end" );
}

void ENC28J60::Init_PHY()
{
	// LANポートのLED設定
	// PHLCON に書く値 low 8bit, LEDB blink fast
	// PHLCON に書く値 high 8bit, LEDA blink fast
	//UART_Print( "PHY LED Setting" );
	//WritePHYReg( PHLCON, 0x0A, 0xA2 );
}

void ENC28J60::Init_PktBuffer()
{
	// Rx buffer の範囲を指定
	WriteCR( ERXSTL, RX_BUFFER_START & 0xFF );
	WriteCR( ERXSTH, RX_BUFFER_START >> 8 );
	WriteCR( ERXNDL, RX_BUFFER_END & 0xFF );
	WriteCR( ERXNDH, RX_BUFFER_END >> 8 );
	// Rx buffer の読み取りポインタ初期化
	WriteCR( ERXRDPTL, RX_BUFFER_START & 0xFF );
	WriteCR( ERXRDPTH, RX_BUFFER_START >> 8 );
	// Tx buffer の範囲を指定
	WriteCR( ETXSTL, TX_BUFFER_START & 0xFF );
	WriteCR( ETXSTH, TX_BUFFER_START >> 8 );
	WriteCR( ETXNDL, TX_BUFFER_END & 0xFF );
	WriteCR( ETXNDH, TX_BUFFER_END >> 8 );
}

void ENC28J60::Init_PktFilter()
{
	// CECEN: CRC validity check Enable
	// packet filter はとりあえず使用しない
	WriteCR( ERXFCON, 0x20 );
}

void ENC28J60::Init_MAC()
{
	// TXPAUS, RXPAUS: pause frame の送信受信を有効
	// MARXEN: MACのフレーム受信を有効
	WriteCR( MACON1, 0x0D );
	// PADCFG [001], TXCRCEN :  short frame は 60byte + CRC にパディング
	// FRMLNEN: Frame length check Enable
	// FULDPX: MAC FullDuplex Mode
	WriteCR( MACON3, 0x33 );
	// 受信する最大フレーム長を設定
	WriteCR( MAMXFLL, MAX_FRAME_LENGTH >> 8 );
	WriteCR( MAMXFLH, MAX_FRAME_LENGTH & 0xFF );
	// back-to-back inter-packet gap 設定
	WriteCR( MABBIPG, 0x15 );
	WriteCR( MAIPGL, 0x12 );

	// MAC Address の設定
	WriteCR( MAADR1, m_Settings.macaddr[0] );
	WriteCR( MAADR2, m_Settings.macaddr[1] );
	WriteCR( MAADR3, m_Settings.macaddr[2] );
	WriteCR( MAADR4, m_Settings.macaddr[3] );
	WriteCR( MAADR5, m_Settings.macaddr[4] );
	WriteCR( MAADR6, m_Settings.macaddr[5] );
}

void ENC28J60::Init_Interrupt()
{
	// 割り込み有効
	EnableTxRxErrorInterrupt();
	EnableRecvPktInterrupt();

	UART_Print( "Interrupt Setting" );
}

void ENC28J60::Init_EthControl()
{
	// 受信パケットの受付を有効
	BitFieldSet( ECON1, (1 << 2) );
}

void ENC28J60::Show_Setting()
{
	UART_Print( "ENC28J60 Setting Info" );

	UART_Print( "** LED Setting" );
	uint8_t phlcon[2];
	ReadPHYReg( PHLCON, &phlcon[0], &phlcon[1] );
	UART_HexPrint( phlcon, 2 ); UART_NewLine();

	UART_Print( "** Max Frame Length" );
	uint16_t max_frame_len = ReadCR(MAMXFLL);
	max_frame_len |= ReadCR(MAMXFLH) << 8;
	UART_HexPrint( reinterpret_cast<const uint8_t*>(&max_frame_len), 2 ); UART_NewLine();

	UART_Print( "** MAC Address" );
	uint8_t macaddr[6];
	macaddr[0] = ReadCR(MAADR1);
	macaddr[1] = ReadCR(MAADR2);
	macaddr[2] = ReadCR(MAADR3);
	macaddr[3] = ReadCR(MAADR4);
	macaddr[4] = ReadCR(MAADR5);
	macaddr[5] = ReadCR(MAADR6);
	UART_HexPrint( macaddr, 6 ); UART_NewLine();

	UART_Print( "** Interrupt setting" );
	uint8_t eie = ReadCR(EIE);
	UART_HexPrint( &eie, 1 ); UART_NewLine();
}

void ENC28J60::EnableTxRxErrorInterrupt()
{
	WriteCR( EIE, (INT_ENABLE | TXERIF | RXERIF) );
}

void ENC28J60::EnableRecvPktInterrupt()
{
	WriteCR( EIE, (INT_ENABLE | PKTIF) );
}

void ENC28J60::DisableInterrupt()
{
	WriteCR( EIE, INT_ENABLE );
}

