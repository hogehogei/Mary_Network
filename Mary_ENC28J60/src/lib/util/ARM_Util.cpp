/*
 * ARM_Util.cpp
 *
 *  Created on: 2019/03/10
 *      Author: hogehogei
 */

#include "lib/util/ARM_Util.hpp"

uint32_t Get_StackPointer_Address()
{
	uint32_t stackpointer = 0;
	asm volatile (
			"mov %[stptr], r13;"
			: [stptr] "+r" (stackpointer)
			:
			:);

	return stackpointer;
}


