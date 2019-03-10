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
#include "uart.h"
#include "timer32.h"
#include "initialize.hpp"
#include "network.h"
#include "lib/util/Endian.hpp"
#include "lib/memory/allocator.hpp"
#include "lib/net/link_layer.hpp"
#include "lib/net/arp_resolver.hpp"
#include "lib/net/internet_layer.hpp"
#include "lib/net/icmp_client.hpp"

using ByteOrder = exlib::Endian<exlib::BigEndian>;

// TODO: insert other definitions and declarations here

// プロトタイプ宣言
void Update_1ms_Timer();
void Update_LED_Indicator();
void Update_Network_Rx();
void Update_ICMP_Send();
void Update_ARP();
void Update_Network_Tx();

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
int g_LED_Idx = 0;

uint32_t g_PacketRecvTimer = 0;
uint32_t g_PingSendTimer = 0;
uint32_t g_ARP_UpdateTimer = 0;
uint32_t g_Ping_TargetIP = 0;

int main(void) {

    // TODO: insert code here

	Initialize_CPU();
	Initialize_Peripheral();
	Call_Static_Initializers();

	Timer32B1_SetCallback( Update_1ms_Timer );	// コールバック設定
    Initialize_Allocator();						// メモリアロケータ初期化
	Initialize_Network();						// ネットワーク初期化

	TurnOnLED( skLEDColorTbl[g_LED_Idx] );

	uint8_t tmp[] = { 192, 168, 24, 128 };
	g_Ping_TargetIP = ByteOrder::GetUint32( tmp );

	// main loop
	while(1){

		Update_LED_Indicator();
		Update_Network_Rx();
		Update_ICMP_Send();
		Update_ARP();
		Update_Network_Tx();

		asm( "wfi" );
	}

    return 0 ;
}

void Update_1ms_Timer(void)
{
	++g_PacketRecvTimer;
	++g_PingSendTimer;
	++g_LEDCount;
	++g_ARP_UpdateTimer;
}

void Update_LED_Indicator()
{
	if( g_LEDCount >= 1000 ){
		g_LEDCount = 0;
		TurnOffLED( skLEDColorTbl[g_LED_Idx] );
		g_LED_Idx = (g_LED_Idx + 1) % sk_LEDColorTbl_Size;
		TurnOnLED( skLEDColorTbl[g_LED_Idx] );
	}
}

void Update_Network_Rx()
{
	if( g_PacketRecvTimer >= 10 ){
		g_PacketRecvTimer = 0;

		LinkLayer& l2 = LinkLayer::Instance();
		l2.Recv_AllInterface();
	}
}

void Update_ICMP_Send()
{
	if( g_PingSendTimer >= 1000 ){
		UART_Print( "Send Ping" );
		g_PingSendTimer = 0;

		ICMP_Client& icmp = ICMP_Client::Instance();
		icmp.Ping( g_Ping_TargetIP );
	}
}

void Update_ARP()
{
	if( g_ARP_UpdateTimer >= 10 ){
		ARP_Resolver& arp_resolver = ARP_Resolver::Instance();
		arp_resolver.Update();
	}
}

void Update_Network_Tx()
{
	InternetLayer& l3 = InternetLayer::Instance();
	l3.SendQueue_Process();
}
