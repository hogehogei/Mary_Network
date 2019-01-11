/*
 * ethernet.hpp
 *
 *  Created on: 2018/12/01
 *      Author: hogehogei
 */

#ifndef LIB_NET_ETHERNET_HPP_
#define LIB_NET_ETHERNET_HPP_

#include <cstdint>

constexpr uint16_t k_EthType_ARP	= 0x0806;
constexpr uint16_t k_EthType_IPv4	= 0x0800;

// network パケットの構造体
// なぜ 16/32bit の値を使わないかというと
// CortexM0 はUnalign Memory Access が一切できない
// つまり 16bit なら 16bit境界, 32bit なら32bit境界にデータを置く必要があるけど
// ネットワークのヘッダを順番に並べるとどうしてもそうならない
// なのでバイト境界を問わない8bit値ですべてアクセスするようにするしかない・・・・・・
// 他の用途ならパディングしておけばOKなはず
struct Ether_Hdr {
	uint8_t macdst[6];
	uint8_t macsrc[6];
	uint8_t type[2];
	uint8_t data[0];
};

class Ethernet
{
public:

	Ethernet( uint8_t* ethhdr );

	const uint8_t* DstMacAddr() const;
	const uint8_t* SrcMacAddr() const;
	void DstMacAddr( const uint8_t* macaddr );
	void SrcMacAddr( const uint8_t* macaddr );

	uint16_t Type() const;
	void Type( uint16_t type );

	const uint8_t* Data() const;
	uint8_t* Data();

private:

	Ether_Hdr* m_EthHdr;
};

void SetMacAddr( const uint8_t* src, uint8_t* dst );
bool IsSameMacAddr( const uint8_t* mac_a, const uint8_t* mac_b );
bool IsBroadCastMacAddr( const uint8_t* macaddr );

#endif /* LIB_NET_ETHERNET_HPP_ */
