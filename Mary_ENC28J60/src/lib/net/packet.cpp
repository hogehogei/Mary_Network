/*
 * eth_frame.cpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#include <lib/net/packet.hpp>

constexpr uint32_t k_HeaderSize = 14;
constexpr uint32_t k_MacAddrLen = 6;

constexpr uint32_t k_DstMacPos  = 0;
constexpr uint32_t k_SrcMacPos  = 6;
constexpr uint32_t k_TypePos    = 12;

Packet::Packet()
: m_RawPacket( nullptr ),
  m_Len( 0 )
{}

Packet::Packet( uint16_t datasize )
: m_RawPacket( new uint8_t[datasize + k_HeaderSize] ),
  m_Len( datasize + k_HeaderSize )
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

const uint8_t* Packet::Data() const
{
	return m_RawPacket + k_HeaderSize;
}

uint8_t* Packet::Data()
{
	return m_RawPacket + k_HeaderSize;
}

uint16_t Packet::Size() const
{
	return m_Len;
}

uint16_t Packet::DataSize() const
{
	return m_Len - k_HeaderSize;
}

PacketPtr Create_Packet( uint16_t datasize )
{
	return PacketPtr( new Packet(datasize) );
}


