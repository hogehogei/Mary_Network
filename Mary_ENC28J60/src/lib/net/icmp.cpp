/*
 * icmp.cpp
 *
 *  Created on: 2018/12/03
 *      Author: hogehogei
 */

#include "lib/net/icmp.hpp"
#include "lib/net/ipv4.hpp"
#include "lib/util/Endian.hpp"


using ByteOrder = exlib::Endian<exlib::BigEndian>;


ICMP::ICMP( uint8_t* icmphdr, uint16_t datalen )
 : m_ICMP_Hdr( reinterpret_cast<ICMP_Hdr*>(icmphdr) ),
   m_DataLen( datalen )
{}

uint8_t ICMP::Type() const
{
	return m_ICMP_Hdr->type;
}

void ICMP::Type( uint8_t type )
{
	m_ICMP_Hdr->type = type;
}

uint8_t ICMP::Code() const
{
	return m_ICMP_Hdr->code;
}

void ICMP::Code( uint8_t code )
{
	m_ICMP_Hdr->code = code;
}

uint16_t ICMP::ChkSum() const
{
	return ByteOrder::GetUint16( m_ICMP_Hdr->chksum );
}

void ICMP::CalculateChkSum()
{
	ByteOrder::SetUint16( m_ICMP_Hdr->chksum, 0x0000 );		// checksum 0初期化してから計算
	uint16_t chksum = CalculateNetworkChkSum( reinterpret_cast<uint8_t*>(m_ICMP_Hdr), sizeof(ICMP_Hdr) + m_DataLen );
	ByteOrder::SetUint16( m_ICMP_Hdr->chksum, chksum );
}

const uint8_t* ICMP::Data() const
{
	return m_ICMP_Hdr->data;
}

uint8_t* ICMP::Data()
{
	return m_ICMP_Hdr->data;
}
