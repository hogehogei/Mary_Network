/*
 * eth_frame.cpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#include <lib/net/packet.hpp>
#include "uart.h"
#include "led.h"

Packet::Packet()
: m_RawPacket( nullptr ),
  m_Len( 0 )
{}

Packet::Packet( uint16_t datasize, uint8_t* datap )
: m_RawPacket( datap ),
  m_Len( datasize )
{}

Packet::~Packet()
{
	delete [] m_RawPacket;
}

const uint8_t* Packet::Head() const
{
	return m_RawPacket;
}

uint8_t* Packet::Head()
{
	return m_RawPacket;
}

IPv4 Packet::Get_IPv4()
{
	return IPv4( m_RawPacket + sizeof(Ether_Hdr) );
}

Ethernet Packet::Get_Eth()
{
	return Ethernet( m_RawPacket );
}

ARP Packet::Get_ARP()
{
	return ARP( m_RawPacket + sizeof(Ether_Hdr) );
}

ICMP Packet::Get_ICMP()
{
	uint16_t payload = m_Len - (sizeof(Ether_Hdr) + sizeof(IPv4_Hdr) + sizeof(ICMP_Hdr));
	return ICMP( m_RawPacket + sizeof(Ether_Hdr) + sizeof(IPv4_Hdr), payload );
}

uint16_t Packet::Size() const
{
	return m_Len;
}


void PrintPacket( const PacketPtr& packet )
{
	if( packet.isNull() ){
		return;
	}

	uint32_t size = packet->Size();
	const uint8_t* p = packet->Head();
	const char space = ' ';

	for( uint32_t i = 0; i < size; ++i ){
		if( i > 0 && ((i & 0x0F) == 0) ){
			UART_NewLine();
		}
		UART_HexPrint( p, 1 );
		UART_PrintChar( space );

		++p;
	}

	UART_NewLine();
}

PacketPtr Create_Packet( uint32_t packet_size )
{
	uint8_t* p = new uint8_t[packet_size];

	if( p != nullptr ){
		return PacketPtr( new Packet( packet_size, p ) );
	}

	return PacketPtr();
}

PacketPtr Create_ARP_Packet()
{
	uint32_t pkt_size = sizeof(Ether_Hdr) + sizeof(ARP_Hdr);

	uint8_t* p = new uint8_t[pkt_size];

	if( p != nullptr ){
		return PacketPtr( new Packet( pkt_size, p ) );
	}

	return PacketPtr();
}

PacketPtr Create_ICMP_Packet( uint32_t payload )
{
	uint32_t pkt_size = sizeof(Ether_Hdr) + sizeof(IPv4_Hdr) + sizeof(ICMP_Hdr) + payload;

	uint8_t* p = new uint8_t[pkt_size];
	if( p != nullptr ){
		return PacketPtr( new Packet( pkt_size, p ) );
	}

	return PacketPtr();
}

