/*
 * gpio.cpp
 *
 *  Created on: 2018/11/18
 *      Author: hogehogei
 */

#include "drv/gpio.hpp"

GPIO::GPIO()
 : mGPIO_Port( nullptr ),
   mBitPos( 1 ),
   mPortLogic( PORT_LOGIC_H_ON )
{}

GPIO::GPIO( volatile uint32_t* port, uint8_t bitpos, PortLogic logic )
 : mGPIO_Port( port ),
   mBitPos( (1 << bitpos) ),
   mPortLogic( logic )
{}

GPIO::~GPIO()
{}

void GPIO::On()
{
	if( mPortLogic == PORT_LOGIC_H_ON ){
		Hi();
	}
	else {
		Lo();
	}
}

void GPIO::Off()
{
	if( mPortLogic == PORT_LOGIC_H_ON ){
		Lo();
	}
	else {
		Hi();
	}
}

void GPIO::Hi()
{
	*mGPIO_Port |= mBitPos;
}

void GPIO::Lo()
{
	*mGPIO_Port &= ~mBitPos;
}

void GPIO::Set( bool on_off )
{
	if( on_off == true ){
		On();
	}
	else {
		Off();
	}
}

void GPIO::Invert()
{
	if( isOn() == true ){
		Off();
	}
	else {
		On();
	}
}

bool GPIO::isOn() const
{
	bool result = true;

	if( mPortLogic == PORT_LOGIC_H_ON ){
		result = isHi();
	}
	else {
		result = isLo();
	}

	return result;
}

bool GPIO::isOff() const
{
	return !isOn();
}

bool GPIO::isHi() const
{
	return (*mGPIO_Port & mBitPos) == mBitPos;
}

bool GPIO::isLo() const
{
	return !isHi();
}
