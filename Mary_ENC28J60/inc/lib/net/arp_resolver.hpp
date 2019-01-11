/*
 * arp_resolver.hpp
 *
 *  Created on: 2018/12/08
 *      Author: hogehogei
 */

#ifndef LIB_NET_ARP_RESOLVER_HPP_
#define LIB_NET_ARP_RESOLVER_HPP_


#include <cstdint>
#include "lib/util/FixedQueue.hpp"
#include "lib/net/packet.hpp"


constexpr uint32_t k_ARPTbl_Size = 10;
constexpr uint32_t k_ARP_RequestQueue_Size = 10;

constexpr uint32_t k_ARPRequest_TimeOut = 6000;
constexpr uint32_t k_ARPRequest_SendInterval = 100;

struct ARP_RequestInfo
{
	uint32_t	tbl_delete_timer;
	uint32_t	arp_request_timer;
	uint32_t	dst_ipaddr;
	bool 		is_complete;
};

struct ARP_Entry
{
	uint32_t	ipaddr;
	uint8_t 	hwaddr[6];
};

struct ARP_ResolveResult
{
	uint32_t	ipaddr;
	uint8_t		hwaddr[6];
	bool		is_resolved;
};

class ARP_Table
{
public:

	ARP_ResolveResult Resolve( uint32_t ipaddr );
	void Register( uint32_t ipaddr, const uint8_t* hwaddr );

private:

	bool search_ARPEntry( uint32_t ipaddr, int* idx );


	exlib::FixedQueue<ARP_Entry, k_ARPTbl_Size> m_ARP_Tbl;
};

class ARP_RequestQueue
{
public:

	void Register( uint32_t ipaddr );
	void Remove( uint32_t ipaddr );
	void Update();

private:

	void send_ARP_Request( const ARP_RequestInfo& req );
	bool isRegistered_Request( uint32_t ipaddr );

	exlib::FixedQueue<ARP_RequestInfo, k_ARP_RequestQueue_Size>	m_ARP_RequestQueue;
};

class ARP_Resolver
{
public:

	static ARP_Resolver& Instance();
	ARP_Resolver( const ARP_Resolver& ) = delete;
	ARP_Resolver& operator=( const ARP_Resolver& ) = delete;

	ARP_ResolveResult Resolve( uint32_t src_ipaddr, uint32_t dst_ipaddr );

	bool Process_ARP_Packet( const PacketPtr& arppkt );
	void Update();

private:

	ARP_Resolver();

	void process_ARP_Request( const PacketPtr& arppkt );

	ARP_Table			m_ARP_Tbl;
	ARP_RequestQueue	m_ARP_Request;
};

PacketPtr Create_ARP_Request( const uint8_t* srcmac, uint32_t srcip, uint32_t dstip );
PacketPtr Create_ARP_Reply( const uint8_t* src_macaddr,
							uint32_t src_ipaddr,
							const uint8_t* dst_macaddr,
							uint32_t dst_ipaddr
							);


#endif /* LIB_NET_ARP_RESOLVER_HPP_ */
