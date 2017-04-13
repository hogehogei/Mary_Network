#include "LPC1100.h"
#include "led.h"
#include "timer32.h"
#include "uart.h"
#include "ledarray.h"
#include "systick.h"
#include "spi.h"
#include "ENC28J60.h"
#include "network.h"

#include <NXP/crp.h>
// Code Read Protection 設定用
// この位置のメモリにある値を書き込むとFlashがロックされて
// 2度と書き換えができなくなるらしい。その防止用
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

/* Section boundaries defined in linker script */
extern long _data[], _etext[], _edata[], _bss[], _ebss[];//, _endof_sram[];
extern long _vStackTop[];

static const uint32_t skLEDColorTbl[] = {
		LED_COLOR_RED,
		LED_COLOR_GREEN,
		LED_COLOR_BLUE,
		LED_COLOR_YELLOW,
		LED_COLOR_AQUA,
		LED_COLOR_FUCHSIA,
		LED_COLOR_WHITE,
};


//
// エントリポイント
//
int ResetISR (void)
{
	long *d = 0, *s = 0;
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

	/* Initialize .data/.bss section and static objects get ready to use after this process */
	for (s = _etext, d = _data; d < _edata; *d++ = *s++) ;
	for (d = _bss; d < _ebss; *d++ = 0) ;

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
	// SPI0の初期化
	Init_SPI0( 8 );    // 8bit length
	UART_Print( "Init_SPI()" );
	// ENC28J60 Eth Controller の初期化
	Init_ENC28J60();
	UART_Print( "Init_ENC28J60()" );

	int led_idx = 0;
	TurnOnLED( skLEDColorTbl[led_idx] );

	uint32_t src_ip = ((192) << 24 | (168) << 16 | (20) << 8 | 150);
	uint32_t dst_ip = ((192) << 24 | (168) << 16 | (20) << 8 | 50);

	Packet* pkt = Create_ICMPEchoRequest( src_ip, dst_ip );
	// main loop
	while(1){
		Systick_Wait( 1000 );

		if( Is_LinkUP_ENC28J60() ){
			UART_Print( "Send ICMP echo request" );
			SendPacket_ENC28J60( pkt );
		}
		else {
			// リンクアップしてない！
			UART_Print( "PHY is not linkup." );
		}

		TurnOffLED( skLEDColorTbl[led_idx] );
		led_idx = (led_idx + 1) % (sizeof(skLEDColorTbl) / 4);
		TurnOnLED( skLEDColorTbl[led_idx] );
	}
}

/*--------------------------------------------------------------------/
/ Exception Vector Table                                              /
/--------------------------------------------------------------------*/

void trap (void)
{
	static uint32_t cnt = 0;
	TurnOffLED( LED_COLOR_RED | LED_COLOR_GREEN | LED_COLOR_BLUE );

	while(1){
		++cnt;
		if( (cnt >> 16) & 0x01 ){
			TurnOnLED( LED_COLOR_WHITE );
		}
		else {
			TurnOffLED( LED_COLOR_WHITE );
		}
	}

	/* Trap spurious interrupt */
}

void HardFault_Handler(void)
{
	static uint32_t cnt = 0;
	TurnOffLED( LED_COLOR_RED | LED_COLOR_GREEN | LED_COLOR_BLUE );

	while(1){
		++cnt;
		if( (cnt >> 16) & 0x01 ){
			TurnOnLED( LED_COLOR_RED );
		}
		else {
			TurnOffLED( LED_COLOR_RED );
		}
	}
}

void* const vector[] __attribute__ ((section(".isr_vector"))) =	/* Vector table to be allocated to address 0 */
{
	&_vStackTop,//_endof_sram,	/* Reset value of MSP */
	ResetISR,			/* Reset entry */
	trap,//NMI_Handler,
	HardFault_Handler,//HardFault_Hander,
	0, 0, 0, 0, 0, 0, 0,//<Reserved>
	trap,//SVC_Handler,
	0, 0,//<Reserved>
	trap,//PendSV_Handler,
	SysTick_Handler,//SysTick_Handler,
	trap,//PIO0_0_IRQHandler,
	trap,//PIO0_1_IRQHandler,
	trap,//PIO0_2_IRQHandler,
	trap,//PIO0_3_IRQHandler,
	trap,//PIO0_4_IRQHandler,
	trap,//PIO0_5_IRQHandler,
	trap,//PIO0_6_IRQHandler,
	trap,//PIO0_7_IRQHandler,
	trap,//PIO0_8_IRQHandler,
	trap,//PIO0_9_IRQHandler,
	trap,//PIO0_10_IRQHandler,
	trap,//PIO0_11_IRQHandler,
	trap,//PIO1_0_IRQHandler,
	trap,//C_CAN_IRQHandler,
	trap,//SPI1_IRQHandler,
	trap,//I2C_IRQHandler,
	trap,//CT16B0_IRQHandler,
	trap,//CT16B1_IRQHandler,
	trap,//CT32B0_IRQHandler,
	CT32B1_IRQHandler,//CT32B1_IRQHandler,
	trap,//SPI0_IRQHandler,
	UART_IRQHandler,  //UART_IRQHandler,
	0, 0,//<Reserved>
	trap,//ADC_IRQHandler,
	trap,//WDT_IRQHandler,
	trap,//BOD_IRQHandler,
	0,//<Reserved>
	trap,//PIO_3_IRQHandler,
	trap,//PIO_2_IRQHandler,
	trap,//PIO_1_IRQHandler,
	trap //PIO_0_IRQHandler
};


