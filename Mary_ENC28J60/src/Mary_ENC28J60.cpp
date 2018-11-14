/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include <cstdint>
#include "LPC1100.h"
#include "led.h"
#include "timer32.h"
#include "uart.h"
#include "systick.h"
#include "spi.h"
#include "ENC28J60.h"
#include "network.h"

// TODO: insert other definitions and declarations here

static const uint32_t skLEDColorTbl[] = {
		LED_COLOR_RED,
		LED_COLOR_GREEN,
		LED_COLOR_BLUE,
		LED_COLOR_YELLOW,
		LED_COLOR_AQUA,
		LED_COLOR_FUCHSIA,
		LED_COLOR_WHITE,
};

Packet* gRxPkt = 0;
int gARPRequestCompleted = 0;
int gIsRecvPkt = 0;
int gSendPktWaitCount = 0;
int gLEDCount = 0;

Host gHostSrc;
Host gHostDst;
int  gIsSetSrcHostInfo = 1;

static void Init_HostInfo(void)
{
	Host src = {
			{ 0, },
			{ 192, 168, 24, 150 },
			24   // 255.255.255.0
	};
	gHostSrc = src;

	Host dst = {
			{ 0, },
			{ 192, 168, 24, 50 },
			0
	};
	gHostDst = dst;

	const uint8_t* srcmac = Get_MACAddr_ENC28J60();
	int i = 0;
	for( i = 0; i < 6; ++i ){
		gHostSrc.macaddr[i] = srcmac[i];
	}
}

static int CheckPacketAddressDestination( const Packet* pkt, const Host* srchost )
{
	const Ether_Hdr* ethhdr = (const Ether_Hdr*)pkt->data;

	if( !IsSameMacAddr( srchost->macaddr, ethhdr->macdst ) &&
		!IsBroadCastMacAddr( ethhdr->macdst )
	){
		// macaddressが違うので受け付けない
		return 0;
	}

	uint16_t type = Get_BEU16( ethhdr->type );
	if( type == 0x0800 ){
		const IP_Hdr* iphdr = (const IP_Hdr*)ethhdr->data;
		uint32_t netmask = (0xFFFFFFFF << (32 - srchost->netmask));
		uint32_t netaddr_own = Get_BEU32( srchost->ipaddr ) & netmask;
		uint32_t netaddr_pkt = Get_BEU32( iphdr->dst_addr ) & netmask;
		uint32_t hostaddr_own = Get_BEU32( srchost->ipaddr ) & ~netmask;
		uint32_t hostaddr_pkt = Get_BEU32( iphdr->dst_addr ) & ~netmask;
		if( netaddr_own != netaddr_pkt && netaddr_pkt != netmask ){
			// 違うネットワークなので受け付けない
			return 0;
		}
		if( hostaddr_own != hostaddr_pkt ){
			if( (hostaddr_pkt | netmask) != 0xFFFFFFFF ){
				// ブロードキャストアドレスじゃないので受け付けない
				return 0;
			}
		}
	}

	return 1;
}

static void Show_ICMPEchoReply( const Packet* pkt )
{
	UART_Print( "Receive ICMP Echo Reply" );
}

void SendPacketTimer(void)
{
	++gSendPktWaitCount;
	++gLEDCount;
}



int main(void) {

    // TODO: insert code here

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
	Timer32B1_SetCallback( SendPacketTimer );
	UART_Print( "Init_Timer32B1()" );
	// SPI0の初期化
	Init_SPI0( 8 );    // 8bit length
	UART_Print( "Init_SPI()" );

	// Mary側の割り込み PIO0_3 を割り込み判定に使用
	// High->Low falling edge 時に検出
	GPIO0IS  &= ~_BV(3);
	GPIO0IBE &= ~_BV(3);
	GPIO0IEV &= ~_BV(3);
	GPIO0IE  |= _BV(3);
	__enable_irqn( PIO_0_IRQn );

	// ENC28J60 Eth Controller の初期化
	Init_ENC28J60();
	UART_Print( "Init_ENC28J60()" );
	Init_HostInfo();

	int led_idx = 0;
	TurnOnLED( skLEDColorTbl[led_idx] );

	// main loop
	while(1){
		while( gSendPktWaitCount <= 1000 && !gIsRecvPkt ){
			asm( "wfi" );
		}

		// 受信パケットがある？
		while( Get_RemainPacketCount() > 0 ){
			if( RecvPacket_ENC28J60( &gRxPkt ) == RECV_VALIDPKT &&
				CheckPacketAddressDestination( gRxPkt, &gHostSrc ) ){
				if( IsARPReply( gRxPkt ) ){
					UART_Print( "Receive ARP reply" );
					Process_ARPReply( gRxPkt, &gHostDst );
					gARPRequestCompleted = 1;
				}
				else if( IsICMPEchoReply( gRxPkt ) ){
					Show_ICMPEchoReply( gRxPkt );
				}
			}

			Free_RxPktBuf_ENC28J60( gRxPkt );
			gRxPkt = 0;
		}
		if( Get_RemainPacketCount() == 0 ){
			// パケットを受信しつくしたので割り込み有効に
			EnableRecvPktInterrupt_ENC28J60();
			gIsRecvPkt = 0;
		}

		if( gSendPktWaitCount > 1000 ){
			Packet* pkt = 0;
			if( !gARPRequestCompleted ){
				pkt = Create_ARPRequest( &gHostSrc, &gHostDst );
				UART_Print( "Send ARP request" );
			}
			else {
				// ping 送信
				pkt = Create_ICMPEchoRequest( &gHostSrc, &gHostDst );
				UART_Print( "Send ICMP echo request" );
			}

			SendPacket_ENC28J60( pkt );
			Free_TxPktBuf_ENC28J60( pkt );
			gSendPktWaitCount = 0;
		}

		if( gLEDCount > 1000 ){
			TurnOffLED( skLEDColorTbl[led_idx] );
			led_idx = (led_idx + 1) % (sizeof(skLEDColorTbl) / 4);
			TurnOnLED( skLEDColorTbl[led_idx] );
			gLEDCount = 0;
		}
	}

    return 0 ;
}

__attribute__ ((section(".after_vectors")))
void RecvPacket(void)
{
	DisableInterrupt_ENC28J60();
	int status = InterruptCallback_ENC28J60();

	if( status & INT_LINKCHANGE ){
		UART_Print( "PHY link status change" );
	}
	if( status & INT_RECVPKT ){
		gIsRecvPkt = 1;
	}
	if( status & INT_RXERROR ){
		UART_Print( "RxError, reset rx buffer" );
	}
	if( status & INT_TXERROR ){
		UART_Print( "TxError, reset tx buffer" );
	}

	GPIO0IC |= _BV(3);
	// ここではパケット受信割り込みは有効にしない
	// パケットをすべて受信してから有効にする
	EnableTxRxErrorInterrupt_ENC28J60();
}
