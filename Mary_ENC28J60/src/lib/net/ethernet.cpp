/*
 * ethernet.cpp
 *
 *  Created on: 2018/12/02
 *      Author: hogehogei
 */


#include "lib/net/ethernet.hpp"
#include "lib/util/Endian.hpp"

using ByteOrder = exlib::Endian<exlib::BigEndian>;



Ethernet::Ethernet( uint8_t* ethhdr )
 : m_EthHdr( reinterpret_cast<Ether_Hdr*>(ethhdr) )
{}

const uint8_t* Ethernet::DstMacAddr() const
{
	return m_EthHdr->macdst;
}

const uint8_t* Ethernet::SrcMacAddr() const
{
	return m_EthHdr->macsrc;
}

void Ethernet::DstMacAddr( const uint8_t* macaddr )
{
	SetMacAddr( macaddr, m_EthHdr->macdst );
}

void Ethernet::SrcMacAddr( const uint8_t* macaddr )
{
	SetMacAddr( macaddr, m_EthHdr->macsrc );
}

uint16_t Ethernet::Type() const
{
	return ByteOrder::GetUint16( m_EthHdr->type );
}

void Ethernet::Type( uint16_t type )
{
	ByteOrder::SetUint16( m_EthHdr->type, type );
}

const uint8_t* Ethernet::Data() const
{
	return m_EthHdr->data;
}

uint8_t* Ethernet::Data()
{
	return m_EthHdr->data;
}

void SetMacAddr( const uint8_t* src, uint8_t* dst )
{
	for( int i = 0; i < 6; ++i ){
		dst[i] = src[i];
	}
}

bool IsSameMacAddr( const uint8_t* mac_a, const uint8_t* mac_b )
{
	bool result = true;
	for( int i = 0; i < 6; ++i ){
		if( mac_a[i] != mac_b[i] ){
			result = false;
			break;
		}
	}

	return result;
}

bool IsBroadCastMacAddr( const uint8_t* macaddr )
{
	bool result = true;
	for( int i = 0; i < 6; ++i ){
		if( macaddr[i] != 0xFF ){
			result = false;
			break;
		}
	}

	return result;
}

