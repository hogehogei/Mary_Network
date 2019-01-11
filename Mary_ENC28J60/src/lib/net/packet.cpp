/*
 * eth_frame.cpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#include <lib/net/packet.hpp>


Packet::Packet()
: m_RawPacket( nullptr ),
  m_Len( 0 )
{}

Packet::Packet( uint16_t datasize )
: m_RawPacket( new uint8_t[datasize] ),
  m_Len( datasize )
{

}

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

uint16_t Packet::Size() const
{
	return m_Len;
}


PacketPtr Create_Packet( uint32_t packet_size )
{
	return PacketPtr( new Packet( packet_size ) );
}

PacketPtr Create_ARP_Packet()
{
	return PacketPtr( new Packet(sizeof(Ether_Hdr) + sizeof(ARP_Hdr)) );
}

PacketPtr Create_ICMP_Packet( uint32_t payload )
{
	return PacketPtr( new Packet(sizeof(Ether_Hdr) + sizeof(IPv4_Hdr) + sizeof(ICMP_Hdr) + payload) );
}

