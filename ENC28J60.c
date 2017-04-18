#include "ENC28J60.h"
#include "spi.h"
#include "systick.h"
#include "uart.h"
#include "network.h"

// 現在のbank bank switch するか判定用
static int sCurrentBank = 0;

// 送信用バッファ
static uint8_t sTxPktBuf[1500] = {0};
static Packet  sTxPkt;
static int     sIsTxPktBufInuse = 0;
// 受信用バッファ
static uint8_t sRxPktBuf[1500] = {0};
static Packet  sRxPkt;
static int     sIsRxPktBufInuse = 0;

static uint16_t sNextPacketPtr = RX_BUFFER_START;

// MAC Address
static uint8_t sMACAddr[6] = {
		MACADDR1,
		MACADDR2,
		MACADDR3,
		MACADDR4,
		MACADDR5,
		MACADDR6
};

// Utility Functions
static void BitFieldSet( uint8_t reg, uint8_t val )
{
	CS_Low();
	SPI0_TxRx( _BFS(reg) );
	SPI0_TxRx( val );
	CS_High();
}

static void BitFieldClear( uint8_t reg, uint8_t val )
{
	CS_Low();
	SPI0_TxRx( _BFC(reg) );
	SPI0_TxRx( val );
	CS_High();
}

static void SwitchBank( uint8_t bank )
{
	BitFieldClear( ECON1, 0x03 );
	BitFieldSet( ECON1, bank );
	sCurrentBank = bank;
}

static uint8_t ReadCR( uint8_t reg )
{
	if( GETBANK(reg) != sCurrentBank ){
		SwitchBank( GETBANK(reg) );
	}

	CS_Low();
	uint16_t data;
	SPI0_TxRx( _RCR(reg) );
	data = SPI0_TxRx(0x00);

	if( IS_MAC_MII_REG(reg) ){
		// MAC/MII register の場合は 1バイト目はダミーで2バイト目が有効なデータ
		data = SPI0_TxRx(0x00);
		int i = 0;
		for( i = 0; i < MAC_MII_REGISTER_ACCESS_WAIT; ++i ){
			CS_Low();
		}
	}
	CS_High();

	return data;
}

void WriteCR( uint8_t reg, uint8_t data )
{
	if( GETBANK(reg) != sCurrentBank ){
		SwitchBank( GETBANK(reg) );
	}

	CS_Low();
	SPI0_TxRx( _WCR(reg) );
	SPI0_TxRx( data );

	if( IS_MAC_MII_REG(reg) ){
		int i = 0;
		for( i = 0; i < MAC_MII_REGISTER_ACCESS_WAIT; ++i ){
			CS_Low();
		}
	}
	CS_High();
}

static void ReadPHYReg( uint8_t phy_addr, uint8_t* reg_high, uint8_t* reg_low )
{
	WriteCR( MIREGADR, phy_addr );
	WriteCR( MICMD, 0x01 );    // MII Read Enable bit

	while( ReadCR(MISTAT) & 0x01 ) ; // BUSY の間待つ

	WriteCR( MICMD, 0x00 );    // Clear MII Read Enable bit

	*reg_low = ReadCR( MIRDL );
	*reg_high = ReadCR( MIRDH );
}

static void WritePHYReg( uint8_t phy_addr, uint8_t reg_high, uint8_t reg_low )
{
	WriteCR( MIREGADR, phy_addr );
	WriteCR( MIWRL, reg_low );
	WriteCR( MIWRH, reg_high );
}

static void ReadBufferMem( uint8_t* dst, uint8_t len )
{
	CS_Low();
	// Read buffer memory command
	SPI0_TxRx( _RBM() );
	int i = 0;
	for( i = 0; i < len; ++i ){
		dst[i] = SPI0_TxRx(0x00);
	}
	CS_High();
}

static void WriteBufferMem( const uint8_t* data, uint16_t len )
{
	CS_Low();
	// Write buffer memory command
	SPI0_TxRx( _WBM() );
	int i = 0;
	for( i = 0; i < len; ++i ){
		SPI0_TxRx( data[i] );
	}
	CS_High();
}




void Init_ENC28J60(void)
{
	// システムリセット
	Reset_ENC28J60();
	// PHYの初期化
	Init_PHY_ENC28J60();
	// Pkt Buffer　の初期化
	Init_PktBuffer_ENC28J60();
	// Pkt Filter の初期化
	Init_PktFilter_ENC28J60();
	// MACの初期化
	Init_MAC_ENC28J60();
	// 割り込み設定
	Init_Interrupt_ENC28J60();
	// Ethernet Control 初期化
	Init_EthControl_ENC28J60();

	// デバッグ用
	Show_Setting_ENC28J60();
}

int Is_LinkUP_ENC28J60(void)
{
	uint8_t phstat2[2];
	ReadPHYReg( PHSTAT2, &phstat2[0], &phstat2[1] );

	// PHSTAT2 10bit LSTAT　をチェック
	// PHSTAT2 の Hi側を調べている
	return phstat2[0] & (1 << 2);
}

const uint8_t* Get_MACAddr_ENC28J60(void)
{
	return sMACAddr;
}

Packet* Use_TxPktBuf_ENC28J60(void)
{
	if( sIsTxPktBufInuse ){
		return 0;
	}

	sIsTxPktBufInuse = 1;

	// 先頭の1byteは 送信時の MACコントロール用で、ethernet ヘッダには関係ないので
	// offsetしたアドレスを返す
	sTxPkt.data = sTxPktBuf;
	sTxPkt.len = 0;

	return &sTxPkt;
}

void Free_TxPktBuf_ENC28J60( Packet* packet )
{
	if( packet != (&sTxPkt) ){
		return;
	}
	sIsTxPktBufInuse = 0;
}

Packet* Use_RxPktBuf_ENC28J60(void)
{
	if( sIsRxPktBufInuse ){
		return 0;
	}

	sRxPkt.data = sRxPktBuf;
	sRxPkt.len  = 0;

	return &sRxPkt;
}

void Free_RxPktBuf_ENC28J60( Packet* packet )
{
	if( packet != (&sRxPkt) ){
		return;
	}
	sIsRxPktBufInuse = 0;
}

void SendPacket_ENC28J60( const Packet* packet_out )
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
	uint16_t tx_end_addr = (TX_BUFFER_START + packet_out->len);

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
	WriteBufferMem( packet_out->data, packet_out->len );

	BitFieldSet( ECON1, 0x08 );    // TXRTS: Transmit Request to Send Enable
}

int CopyPacketFromRecvBuffer_ENC28J60( Packet* packet_in )
{
	uint8_t header[6] = {0, };
	int status = 0;

	WriteCR( ERDPTL, sNextPacketPtr & 0xFF );
	WriteCR( ERDPTH, sNextPacketPtr >> 8 );

	ReadBufferMem( header, 6 );    // next packet pointer + status vector を読む
	sNextPacketPtr = header[0] | (header[1] << 8);
	uint16_t recv_byte = header[2] | (header[3] << 8);
	recv_byte -= 4;    // CRC は読まない

	// status vector から CRC Error チェック
	if( header[4] & (1 << 4) ){
		status = RECV_CRCERR;
	}
	else if( !packet_in ){
		// ドロップさせる
		status = RECV_DROPPKT;
	}
	else {
		status = RECV_VALIDPKT;
		// 実際のデータを読む
		ReadBufferMem( packet_in->data, recv_byte );
		packet_in->len = recv_byte;
	}


	// 読み込みポインタを次のアドレスに進める
	// Rev.B4 Errata シート参照
	if( (sNextPacketPtr - 1) < RX_BUFFER_START || (sNextPacketPtr - 1) > RX_BUFFER_END ){
		WriteCR( ERXRDPTL, RX_BUFFER_END & 0xFF );
		WriteCR( ERXRDPTH, RX_BUFFER_END >> 8 );
	}
	else {
		WriteCR( ERXRDPTL, (sNextPacketPtr - 1) & 0xFF );
		WriteCR( ERXRDPTH, (sNextPacketPtr - 1) >> 8 );
	}

	// パケットを処理したので、パケットカウントを1つ減らす
	BitFieldSet( ECON2, (1 << 6) );

	return status;
}

int Get_RemainPacketCount(void)
{
	return ReadCR(EPKTCNT);
}

int RecvPacket_ENC28J60( Packet* packet_in )
{
	if( Get_RemainPacketCount() == 0 ){
		return RECV_NOPKT;
	}

	return CopyPacketFromRecvBuffer_ENC28J60( packet_in );
}

int InterruptCallback_ENC28J60( Packet** packet_in )
{
	int status = 0;
	uint8_t eir = ReadCR(EIR);

	if( eir & PKTIF ){
		if( sIsRxPktBufInuse ){
			// パケットバッファが枯渇してるのでドロップさせる
			*packet_in = 0;
		}
		else {
			*packet_in = Use_RxPktBuf_ENC28J60();
		}
		status |= RecvPacket_ENC28J60( *packet_in );
		// PKTIF は手動ではクリアしない、残パケットなしになったら自動でクリアされる
	}
	if( eir & LINKIF ){
		status |= INT_LINKCHANGE;
		ReadCR( PHIR );
		// LINKIF はPHIRを読むと自動でクリアされる
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

	return status;
}

void EnableInterrupt_ENC28J60(void)
{
	BitFieldSet( EIE, (INT_ENABLE | PKTIF | TXERIF | RXERIF) );
}

void DisableInterrupt_ENC28J60(void)
{
	BitFieldClear( EIE, INT_ENABLE );
}

void Reset_ENC28J60(void)
{
	SPI0_TxRx( _SRC() );
	Systick_Wait( 1 );   // リセット後50us 以上待つ必要があるとのことなので、1ms待つ
	while( !(ReadCR(ESTAT) & 0x01) ) ;    // ClockReady になるまで待つ

	// PHYをリセット
	// PHY FullDuplex Mode
	UART_Print( "Setting Full Duplex Mode" );
	WritePHYReg( PHCON1, 0x81, 0x00 );
	UART_Print( "ENC28J60 reset end" );
}

void Init_PHY_ENC28J60(void)
{
	// LANポートのLED設定
	// PHLCON に書く値 low 8bit, LEDB blink fast
	// PHLCON に書く値 high 8bit, LEDA blink fast
	//UART_Print( "PHY LED Setting" );
	//WritePHYReg( PHLCON, 0x0A, 0xA2 );
}

void Init_PktBuffer_ENC28J60(void)
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

void Init_PktFilter_ENC28J60(void)
{
	// CECEN: CRC validity check Enable
	// packet filter はとりあえず使用しない
	WriteCR( ERXFCON, 0x20 );
}

void Init_MAC_ENC28J60(void)
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
	WriteCR( MAADR1, sMACAddr[0] );
	WriteCR( MAADR2, sMACAddr[1] );
	WriteCR( MAADR3, sMACAddr[2] );
	WriteCR( MAADR4, sMACAddr[3] );
	WriteCR( MAADR5, sMACAddr[4] );
	WriteCR( MAADR6, sMACAddr[5] );
}

void Init_Interrupt_ENC28J60(void)
{
	// 割り込み有効
	//DisableInterrupt_ENC28J60();
	EnableInterrupt_ENC28J60();

	UART_Print( "Interrupt Setting" );
}

void Init_EthControl_ENC28J60(void)
{
	// 受信パケットの受付を有効
	BitFieldSet( ECON1, (1 << 2) );
}

void Show_Setting_ENC28J60(void)
{
	UART_Print( "ENC28J60 Setting Info" );

	UART_Print( "** LED Setting" );
	uint8_t phlcon[2];
	ReadPHYReg( PHLCON, &phlcon[0], &phlcon[1] );
	UART_HexPrint( phlcon, 2 ); UART_NewLine();

	UART_Print( "** Max Frame Length" );
	uint16_t max_frame_len = ReadCR(MAMXFLL);
	max_frame_len |= ReadCR(MAMXFLH) << 8;
	UART_HexPrint( &max_frame_len, 2 ); UART_NewLine();

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
