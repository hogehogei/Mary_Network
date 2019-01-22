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
#include "lib/memory/allocator.hpp"
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
static constexpr uint32_t sk_LEDColorTbl_Size = sizeof(skLEDColorTbl) / sizeof(uint32_t);

int g_LEDCount = 0;

uint32_t g_PacketRecvTimer = 0;
uint32_t g_PingSendTimer = 0;

int  gIsSetSrcHostInfo = 1;

extern unsigned int __init_array_start;
extern unsigned int __init_array_end;
extern void call_static_initializers();

void Update1msTimer(void)
{
	++g_PacketRecvTimer;
	++g_PingSendTimer;
	++g_LEDCount;
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

    call_static_initializers();

    Initialize_Allocator();
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
		}


		if( g_PingSendTimer >= 1000 ){
			//UART_Print( "Send Ping" );
			g_PingSendTimer = 0;

			ICMP_Client& icmp = ICMP_Client::Instance();
			icmp.Ping( target_ipaddr );
		}


		if( g_LEDCount >= 1000 ){
			g_LEDCount = 0;
			TurnOffLED( skLEDColorTbl[led_idx] );
			led_idx = (led_idx + 1) % sk_LEDColorTbl_Size;
			TurnOnLED( skLEDColorTbl[led_idx] );
		}
		asm( "wfi" );
	}

    return 0 ;
}

