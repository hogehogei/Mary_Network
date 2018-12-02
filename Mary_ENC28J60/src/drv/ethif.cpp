/*
 * ethernet.cpp
 *
 *  Created on: 2018/11/25
 *      Author: hogehogei
 */

#include <drv/ethif.hpp>
#include <drv/ethif_drv.hpp>
#include "LPC1100.h"
#include "drv/gpio.hpp"

ENC28J60 EthIf_Drv::m_ENC28J60;
Eth_If EthIf_Drv::m_Eth_If[k_EthIf_Num];
Eth_If EthIf_Drv::m_Invalid_If;

EthIf_Drv::EthIf_Drv()
{}

EthIf_Drv::~EthIf_Drv()
{}

EthIf_Drv::EthIf_Drv( const EthIf_Drv& drv )
{}

EthIf_Drv& EthIf_Drv::operator=( const EthIf_Drv& drv )
{
	return *this;
}

bool EthIf_Drv::Initialize( const Eth_Settings& settings )
{
	if( settings.interface_id > 0 ){
		return false;
	}

	bool result = m_ENC28J60.Initialize( settings,
										 k_ENC28J60_SPI_Ch,
										 GPIO( &GPIO0DIR, 2, GPIO::PORT_LOGIC_L_ON ) );
	m_Eth_If[0] = Eth_If( &m_ENC28J60 );

	return result;
}

Eth_If& EthIf_Drv::Instance( uint8_t interface_id )
{
	if( interface_id > k_EthIf_Num ){
		return m_Invalid_If;
	}

	return m_Eth_If[interface_id];
}

void EthIf_Drv::ENC28J60_ch1_Callback()
{
	m_ENC28J60.Interrupt_Callback();
}






Eth_If::Eth_If()
	: m_Impl( nullptr )
{}

Eth_If::Eth_If( I_EthIf* impl )
	: m_Impl( impl )
{}

Eth_If::~Eth_If()
{}

bool Eth_If::Send( const PacketPtr& frame )
{
	if( m_Impl == nullptr ){
		return false;
	}
	return m_Impl->Send( frame );
}

bool Eth_If::Recv( PacketPtr* frame )
{
	if( m_Impl == nullptr ){
		return false;
	}
	return m_Impl->Recv( frame );
}

uint32_t Eth_If::get_RxRemainPacketCount()
{
	if( m_Impl == nullptr ){
		return 0;
	}
	return m_Impl->get_RxRemainPacketCount();
}

bool Eth_If::isLinkUp()
{
	if( m_Impl == nullptr ){
		return false;
	}
	return m_Impl->isLinkUp();
}

const uint8_t* Eth_If::getMacAddr() const
{
	if( m_Impl == nullptr ){
		return nullptr;
	}
	return m_Impl->getMacAddr();
}



