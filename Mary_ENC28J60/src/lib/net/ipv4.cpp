/*
 * IPv4.cpp
 *
 *  Created on: 2018/12/02
 *      Author: hogehogei
 */

#include "lib/net/ipv4.hpp"
#include "lib/util/Endian.hpp"


using ByteOrder = exlib::Endian<exlib::BigEndian>;


IPv4::IPv4( uint8_t* iphdr )
 : m_IPv4_Hdr( reinterpret_cast<IPv4_Hdr*>(iphdr) )
{}

uint8_t IPv4::Version() const
{
	return ((m_IPv4_Hdr->version_hdrlen >> 4) & 0x0F);
}

void IPv4::Version( uint8_t ver )
{
	uint8_t v = ((ver << 4) & 0xF0) | (m_IPv4_Hdr->version_hdrlen & 0x0F);
	m_IPv4_Hdr->version_hdrlen = v;
}

uint8_t IPv4::HdrLen() const
{
	return (m_IPv4_Hdr->version_hdrlen & 0x0F);
}

void IPv4::HdrLen( uint8_t hdr_len )
{
	uint8_t v = ((m_IPv4_Hdr->version_hdrlen << 4) & 0xF0) | (hdr_len & 0x0F);
	m_IPv4_Hdr->version_hdrlen = v;
}

uint8_t IPv4::Tos() const
{
	return m_IPv4_Hdr->tos;
}

void IPv4::Tos( uint8_t tos )
{
	m_IPv4_Hdr->tos = tos;
}

uint16_t IPv4::TotalLen() const
{
	return ByteOrder::GetUint16( m_IPv4_Hdr->total_len );
}

void IPv4::TotalLen( uint16_t len )
{
	ByteOrder::SetUint16( m_IPv4_Hdr->total_len, len );
}

uint16_t IPv4::Id() const
{
	return ByteOrder::GetUint16( m_IPv4_Hdr->id );
}

void IPv4::Id( uint16_t id )
{
	ByteOrder::SetUint16( m_IPv4_Hdr->id, id );
}

uint8_t IPv4::Flag() const
{
	// 上位3bitをマスクして返す
	return (m_IPv4_Hdr->flag_offset[0] & 0xE0) >> 5;
}

void IPv4::Flag( uint8_t flag )
{
	uint8_t v = ((flag << 5) & 0xE0) | (m_IPv4_Hdr->flag_offset[0] & 0x1F);
	m_IPv4_Hdr->flag_offset[0] = v;
}

uint16_t IPv4::Offset() const
{
	uint16_t v = ByteOrder::GetUint16( m_IPv4_Hdr->flag_offset );
	return (v & 0x1FFF);
}

void IPv4::Offset( uint16_t offset )
{
	uint16_t v = ByteOrder::GetUint16( m_IPv4_Hdr->flag_offset );
	uint16_t newval = (v & 0xE000) | (offset & 0x1FFF);
	ByteOrder::SetUint16( m_IPv4_Hdr->flag_offset, newval );
}

uint8_t IPv4::TTL() const
{
	return m_IPv4_Hdr->ttl;
}

void IPv4::TTL( uint8_t ttl )
{
	m_IPv4_Hdr->ttl = ttl;
}

uint8_t IPv4::Protocol() const
{
	return m_IPv4_Hdr->protocol;
}

void IPv4::Protocol( uint8_t protocol )
{
	m_IPv4_Hdr->protocol = protocol;
}

uint16_t IPv4::ChkSum() const
{
	return ByteOrder::GetUint16( m_IPv4_Hdr->chksum );
}

void IPv4::CalculateChkSum()
{
	ByteOrder::SetUint16( m_IPv4_Hdr->chksum, 0x0000 );		// checksum 0初期化してから計算
	uint16_t chksum = CalculateNetworkChkSum( reinterpret_cast<uint8_t*>(m_IPv4_Hdr), sizeof(IPv4_Hdr) );
	ByteOrder::SetUint16( m_IPv4_Hdr->chksum, chksum );
}

uint32_t IPv4::SrcAddr() const
{
	return ByteOrder::GetUint32( m_IPv4_Hdr->src_addr );
}

void IPv4::SrcAddr( uint32_t addr )
{
	ByteOrder::SetUint32( m_IPv4_Hdr->src_addr, addr );
}

uint32_t IPv4::DstAddr() const
{
	return ByteOrder::GetUint32( m_IPv4_Hdr->dst_addr );
}

void IPv4::DstAddr( uint32_t addr )
{
	ByteOrder::SetUint32( m_IPv4_Hdr->dst_addr, addr );
}


const uint8_t* IPv4::Data() const
{
	return m_IPv4_Hdr->data;
}

uint8_t* IPv4::Data()
{
	return m_IPv4_Hdr->data;
}

uint16_t CalculateNetworkChkSum( const uint8_t* data, uint32_t len )
{
	int remain_size = len;
	const uint8_t* p = data;

	uint32_t sum = 0;
	while( remain_size >  1 ){
		sum += ByteOrder::GetUint16( p );
		p += 2;
		remain_size -= 2;
	}
	// 残り1バイト余ってる？
	if( remain_size ){
		sum += *p;
	}

	sum = (sum & 0xFFFF) + (sum >> 16);
	sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

