/*
 * packet.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef DRV_PACKET_HPP_
#define DRV_PACKET_HPP_

#include <cstdint>
#include "lib/util/RefCount.hpp"
#include "lib/util/IntrusivePtr.hpp"
#include "lib/net/ethernet.hpp"
#include "lib/net/ipv4.hpp"
#include "lib/net/arp.hpp"
#include "lib/net/icmp.hpp"


class Packet : public exlib::RefCount
{
public:

	Packet();
	Packet( uint16_t datasize, uint8_t* datap );
	~Packet() noexcept;

	const uint8_t* Head() const;
	uint8_t* Head();

	IPv4 Get_IPv4();
	Ethernet Get_Eth();
	ARP Get_ARP();
	ICMP Get_ICMP();

	uint16_t Size() const;

private:

	uint8_t* m_RawPacket;
	uint16_t m_Len;
};

using PacketPtr = exlib::IntrusivePtr<Packet>;

void PrintPacket( const PacketPtr& packet );

PacketPtr Create_Packet( uint32_t packet_size );
PacketPtr Create_ARP_Packet();
PacketPtr Create_ICMP_Packet( uint32_t payload );



#endif /* DRV_PACKET_HPP_ */
