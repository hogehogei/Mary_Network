/*
 * global.hpp
 *
 *  Created on: 2018/11/13
 *      Author: hogehogei
 */

#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_

#include <cstdint>

#ifdef GLOBAL_VALUE_DEFINITION
#define		EXTERN
#else
#define		EXTERN	extern
#endif

EXTERN void (*gTimer32B1_Callback)(void);
EXTERN volatile uint32_t gSystick_WaitCalled;
EXTERN volatile uint32_t gSystick_WaitCnt;

#endif /* GLOBAL_HPP_ */
