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
#include "network.h"
#include "ENC28J60.h"

#include "lib/util/Endian.hpp"
#include "lib/net/link_layer.hpp"
#include "lib/net/icmp_client.hpp"

using ByteOrder = exlib::Endian<exlib::BigEndian>;

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

int gLEDCount = 0;

uint32_t g_PacketRecvTimer = 0;
uint32_t g_PingSendTimer = 0;

int  gIsSetSrcHostInfo = 1;

#if 0
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
#endif

static void Show_ICMPEchoReply( const Packet* pkt )
{
	UART_Print( "Receive ICMP Echo Reply" );
}

void Update1msTimer(void)
{
	++g_PacketRecvTimer;
	++g_PingSendTimer;
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
	Timer32B1_SetCallback( Update1msTimer );
	UART_Print( "Init_Timer32B1()" );

	// ネットワークの初期化
	Initialize_Network();

	int led_idx = 0;
	TurnOnLED( skLEDColorTbl[led_idx] );

	uint8_t tmp[] = { 192, 168, 24, 2 };
	uint32_t target_ipaddr = ByteOrder::GetUint32( tmp );
	// main loop
	while(1){

		if( g_PacketRecvTimer >= 10 ){
			g_PacketRecvTimer = 0;

			LinkLayer& l2 = LinkLayer::Instance();
			l2.Recv_AllInterface();
			Recv_ICMP_Reply();
		}

		if( g_PingSendTimer >= 1000 ){
			g_PingSendTimer = 0;

			ICMP_Client& icmp = ICMP_Client::Instance();
			icmp.Ping( target_ipaddr );
		}
		asm( "wfi" );
	}

    return 0 ;
}

