/*
 * global.hpp
 *
 *  Created on: 2018/11/13
 *      Author: hogehogei
 */

#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_


#ifdef GLOBAL_VALUE_DEFINITION
#define		EXTERN
#else
#define		EXTERN	extern
#endif

EXTERN void (*gTimer32B1_Callback)(void);

#endif /* GLOBAL_HPP_ */
