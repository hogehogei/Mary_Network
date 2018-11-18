
#include "systick.h"
#include "led.h"
#include "uart.h"
#include "LPC1100.h"
#include "global.hpp"

int Init_Systick(void)
{
	SYST_RVR = 36*1000 - 1;  // 1ms 毎に割り込み発生
	SYST_CVR = 0;
	SYST_CSR = 0x07;

	gSystick_WaitCalled = 0;
	gSystick_WaitCnt = 0;

	return 1;
}

// 引数の時間（ミリ秒）待つ
void Systick_Wait( uint32_t mill_sec )
{
	gSystick_WaitCalled = 1;
	gSystick_WaitCnt = 0;
	while( gSystick_WaitCnt < mill_sec ) {
		// 次の割り込み開始まで待つ
		asm("wfi");
	}

	gSystick_WaitCalled = 0;
}
