/*
 * arp.cpp
 *
 *  Created on: 2018/12/01
 *      Author: hogehogei
 */

#include "lib/net/arp.hpp"
#include "lib/net/ethernet.hpp"
#include "lib/util/Endian.hpp"


using ByteOrder = exlib::Endian<exlib::BigEndian>;


ARP::ARP( uint8_t* arphdr )
 : m_ARP_Hdr( reinterpret_cast<ARP_Hdr*>(arphdr) )
{}

uint16_t ARP::HwType() const
{
	return ByteOrder::GetUint16( m_ARP_Hdr->hw_type );
}
void ARP::HwType( uint16_t hwtype )
{
	ByteOrder::SetUint16( m_ARP_Hdr->hw_type, hwtype );
}

uint16_t ARP::Protocol() const
{
	return ByteOrder::GetUint16( m_ARP_Hdr->protocol );
}

void ARP::Protocol( uint16_t protocol )
{
	ByteOrder::SetUint16( m_ARP_Hdr->protocol, protocol );
}

uint8_t ARP::Hlen() const
{
	return m_ARP_Hdr->hlen;
}

void ARP::Hlen( uint8_t hlen )
{
	m_ARP_Hdr->hlen = hlen;
}

uint8_t ARP::Plen() const
{
	return m_ARP_Hdr->plen;
}

void ARP::Plen( uint8_t plen )
{
	m_ARP_Hdr->plen = plen;
}

uint16_t ARP::OpCode() const
{
	return ByteOrder::GetUint16( m_ARP_Hdr->op_code );
}

void ARP::OpCode( uint16_t opcode )
{
	ByteOrder::SetUint16( m_ARP_Hdr->op_code, opcode );
}

const uint8_t* ARP::DstMacAddr() const
{
	return m_ARP_Hdr->macdst;
}

const uint8_t* ARP::SrcMacAddr() const
{
	return m_ARP_Hdr->macsrc;
}

void ARP::DstMacAddr( const uint8_t* macaddr )
{
	SetMacAddr( macaddr, m_ARP_Hdr->macdst );
}

void ARP::SrcMacAddr( const uint8_t* macaddr )
{
	SetMacAddr( macaddr, m_ARP_Hdr->macsrc );
}

uint32_t ARP::DstIpAddr() const
{
	return ByteOrder::GetUint32( m_ARP_Hdr->ipdst );
}

uint32_t ARP::SrcIpAddr() const
{
	return ByteOrder::GetUint32( m_ARP_Hdr->ipsrc );
}

void ARP::DstIpAddr( uint32_t ipaddr )
{
	ByteOrder::SetUint32( m_ARP_Hdr->ipdst, ipaddr );
}

void ARP::SrcIpAddr( uint32_t ipaddr )
{
	ByteOrder::SetUint32( m_ARP_Hdr->ipsrc, ipaddr );
}

bool Is_ARP_Packet( const Ethernet& eth )
{
	if( eth.Type() != 0x0806 ){    // ethernet header の type がARPじゃない
		return false;
	}

	return true;
}

bool Is_ARP_Reply( const ARP& arp )
{
	if( arp.HwType() == 0x0001 && arp.Protocol() == 0x0800 && arp.OpCode() == 2 ){
		return true;
	}

	return false;
}

