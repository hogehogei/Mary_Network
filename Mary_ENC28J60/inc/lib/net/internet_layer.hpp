/*
 * InternetLayer.hpp
 *
 *  Created on: 2018/12/07
 *      Author: hogehogei
 */

#ifndef LIB_NET_INTERNET_LAYER_HPP_
#define LIB_NET_INTERNET_LAYER_HPP_

#include <cstdint>
#include "lib/util/FixedQueue.hpp"
#include "lib/net/packet.hpp"


struct Route
{
	uint32_t ipaddr;
	uint32_t netmask;
};

struct RoutingEntry
{
	uint32_t	ipaddr;
	uint32_t	netmask;
	uint32_t	gw;
	uint8_t		metric;
	uint8_t		interface_id;
};

struct IPAddr_BindEntry
{
	uint32_t	ipaddr;
	uint32_t    netmask;
	uint8_t		interface_id;
};

class RoutingTable
{
public:

	static constexpr uint32_t k_RoutingTblSize = 5;

public:

	RoutingTable();
	~RoutingTable();

	bool AddRoute( const RoutingEntry& entry );
	bool SearchRoute( uint32_t dst_ipaddr, RoutingEntry* next_hop ) const;

private:

	RoutingEntry m_Entry[k_RoutingTblSize];
	uint32_t m_RoutingTbl_Last;
};


class InternetLayer
{
private:

	static constexpr uint32_t k_IPAddr_BindTbl_Size = 5;
	static constexpr uint32_t k_SendQueue_Size = 5;

public:

	static InternetLayer& Instance();

	InternetLayer( const InternetLayer& ) = delete;
	InternetLayer& operator=( const InternetLayer& ) = delete;

	bool Bind( uint8_t interface_id, const Route& route );
	bool AddRoute( const RoutingEntry& entry );

	void Send( PacketPtr packet );
	void Recv( uint8_t interface_id, const PacketPtr& packet );

	bool GetInterface_ByIPAddr( uint32_t ipaddr, uint8_t* interface_id ) const;
	bool GetInterface_ByDestRoute( uint32_t dst_ipaddr, uint8_t* interface_id ) const;
	bool GetIPAddr_ByInterface( uint8_t interface_id, uint32_t* ipaddr ) const;
	bool GetNetMask_ByInterface( uint8_t interface_id, uint32_t* netmask ) const;

	void SendQueue_Process();

private:

	InternetLayer();
	bool TrySendPacket( PacketPtr packet );
	bool RxFilter_IPv4Addr( uint8_t interface_id, const PacketPtr& packet );

	IPAddr_BindEntry	m_IPAddr_BindTbl[k_IPAddr_BindTbl_Size];
	uint32_t			m_IPAddr_BindTblIdx;
	RoutingTable		m_RoutingTbl;

	exlib::FixedQueue<PacketPtr, 5>	m_SendQueue;
};



#endif /* LIB_NET_INTERNET_LAYER_HPP_ */
