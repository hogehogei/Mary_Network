/*
 * arp.hpp
 *
 *  Created on: 2018/12/01
 *      Author: hogehogei
 */

#ifndef LIB_NET_ARP_HPP_
#define LIB_NET_ARP_HPP_

#include <cstdint>
#include "lib/net/ethernet.hpp"



constexpr uint16_t k_ARP_Operation_ARP_Request	= 0x0001;
constexpr uint16_t k_ARP_Operation_ARP_Reply	= 0x0002;






struct ARP_Hdr
{
	uint8_t hw_type[2];
	uint8_t protocol[2];
	uint8_t hlen;
	uint8_t plen;
	uint8_t op_code[2];
	uint8_t macsrc[6];
	uint8_t ipsrc[4];
	uint8_t macdst[6];
	uint8_t ipdst[4];
};

class ARP
{
public:

	ARP( uint8_t* arphdr );

	uint16_t HwType() const;
	void HwType( uint16_t hwtype );

	uint16_t Protocol() const;
	void Protocol( uint16_t protocol );

	uint8_t Hlen() const;
	void Hlen( uint8_t hlen );

	uint8_t Plen() const;
	void Plen( uint8_t plen );

	uint16_t OpCode() const;
	void OpCode( uint16_t opcode );

	const uint8_t* DstMacAddr() const;
	const uint8_t* SrcMacAddr() const;
	void DstMacAddr( const uint8_t* macaddr );
	void SrcMacAddr( const uint8_t* macaddr );

	uint32_t DstIpAddr() const;
	uint32_t SrcIpAddr() const;
	void DstIpAddr( uint32_t ipaddr );
	void SrcIpAddr( uint32_t ipaddr );

private:

	ARP_Hdr*	m_ARP_Hdr;
};

bool Is_ARP_Packet( const Ethernet& eth );
bool Is_ARP_Reply( const ARP& arp );


#endif /* LIB_NET_ARP_HPP_ */
