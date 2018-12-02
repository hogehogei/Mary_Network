/*
 * ipv4.hpp
 *
 *  Created on: 2018/12/02
 *      Author: hogehogei
 */

#ifndef LIB_NET_IPV4_HPP_
#define LIB_NET_IPV4_HPP_

#include <cstdint>

struct IPv4_Hdr {
	uint8_t version_hdrlen;
	uint8_t tos;
	uint8_t total_len[2];
	uint8_t id[2];
	uint8_t flag_offset[2];    // flag 3bit, fragment offset, 13bit
	uint8_t ttl;
	uint8_t protocol;
	uint8_t chksum[2];
	uint8_t src_addr[4];
	uint8_t dst_addr[4];
	// option, padding はとりあえず無視！
	uint8_t data[0];
};

class IPv4
{
public:

	IPv4( uint8_t* iphdr );

	uint8_t Version() const;
	void Version( uint8_t ver );

	uint8_t HdrLen() const;
	void HdrLen( uint8_t hdr_len );

	uint8_t Tos() const;
	void Tos( uint8_t tos );

	uint16_t TotalLen() const;
	void TotalLen( uint16_t len );

	uint16_t Id() const;
	void Id( uint16_t id );

	uint8_t Flag() const;
	void Flag( uint8_t flag );

	uint16_t Offset() const;
	void Offset( uint16_t offset );

	uint8_t TTL() const;
	void TTL( uint8_t ttl );

	uint8_t Protocol() const;
	void Protocol( uint8_t protocol );

	uint16_t Chksum() const;
	void CalculateChkSum();

	const uint8_t* SrcAddr() const;
	uint8_t* SrcAddr();

	const uint8_t* DstAddr() const;
	uint8_t* DstAddr();

	const uint8_t* Data() const;
	uint8_t* Data();

private:

	IPv4_Hdr* m_IPv4_Hdr;
};

uint16_t CalculateNetworkChkSum( const uint8_t* data, uint32_t len );


#endif /* LIB_NET_IPV4_HPP_ */
