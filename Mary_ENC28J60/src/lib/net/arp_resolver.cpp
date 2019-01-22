/*
 * arp_resolver.cpp
 *
 *  Created on: 2018/12/08
 *      Author: hogehogei
 */

#include "lib/net/arp_resolver.hpp"
#include "lib/net/internet_layer.hpp"
#include "lib/net/link_layer.hpp"

ARP_ResolveResult ARP_Table::Resolve( uint32_t ipaddr )
{
	ARP_ResolveResult result;
	result.is_resolved = false;

	int idx = 0;
	if( search_ARPEntry( ipaddr, &idx ) ){
		result.ipaddr = ipaddr;
		for( int i = 0; i < 6; ++i ){
			result.hwaddr[i] = m_ARP_Tbl[idx].hwaddr[i];
		}
		result.is_resolved = true;
	}

	return result;
}

void ARP_Table::Register( uint32_t ipaddr, const uint8_t* hwaddr )
{
	ARP_Entry entry;

	entry.ipaddr = ipaddr;
	for( int i = 0; i < 6; ++i ){
		entry.hwaddr[i] = hwaddr[i];
	}

	m_ARP_Tbl.Push( entry );
}

bool ARP_Table::search_ARPEntry( uint32_t ipaddr, int* idx )
{
	bool result = false;
	int size = m_ARP_Tbl.Size();

	for( int i = 0; i < size; ++i ){
		if( m_ARP_Tbl[i].ipaddr == ipaddr ){
			*idx = i;
			result = true;
			break;
		}
	}

	return result;
}


void ARP_RequestQueue::Register( uint32_t ipaddr )
{
	if( !isRegistered_Request( ipaddr ) ){
		ARP_RequestInfo request;
		request.tbl_delete_timer	= 0;
		request.arp_request_timer	= 0;
		request.dst_ipaddr 			= ipaddr;
		request.is_complete			= false;

		m_ARP_RequestQueue.Push( request );
	}
}

void ARP_RequestQueue::Remove( uint32_t ipaddr )
{
	uint32_t size = m_ARP_RequestQueue.Size();

	for( uint32_t i = 0; i < size; ++i ){
		ARP_RequestInfo& req = m_ARP_RequestQueue[i];
		if( ipaddr == req.dst_ipaddr ){
			req.is_complete = true;
		}
	}
}

void ARP_RequestQueue::Update()
{
	uint32_t size = m_ARP_RequestQueue.Size();

	for( uint32_t i = 0; i < size; ++i ){
		ARP_RequestInfo& req = m_ARP_RequestQueue[i];
		if( req.is_complete == false ){
			++req.tbl_delete_timer;
			++req.arp_request_timer;
			if( req.tbl_delete_timer >= k_ARPRequest_TimeOut ){
				req.is_complete = true;
			}
			if( req.arp_request_timer >= k_ARPRequest_SendInterval ){
				req.arp_request_timer = 0;
				send_ARP_Request( req );
			}
		}
	}
}

void ARP_RequestQueue::send_ARP_Request( const ARP_RequestInfo& req )
{
	InternetLayer& l3 = InternetLayer::Instance();
	uint8_t interface_id = 0;
	if( !l3.GetInterface_ByDestRoute( req.dst_ipaddr, &interface_id ) ){
		return;
	}

	uint32_t srcip = 0;
	if( !l3.GetIPAddr_ByInterface( interface_id, &srcip ) ){
		return;
	}

	LinkLayer& l2 = LinkLayer::Instance();
	uint8_t mac_broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	const uint8_t* srcmac = l2.GetMacAddr( interface_id );
	PacketPtr arp_request = Create_ARP_Request( srcmac, srcip, req.dst_ipaddr );
	l2.Send( arp_request, interface_id, mac_broadcast, k_EthType_ARP );
}

bool ARP_RequestQueue::isRegistered_Request( uint32_t ipaddr )
{
	uint32_t size = m_ARP_RequestQueue.Size();

	bool is_found = false;
	for( uint32_t i = 0; i < size; ++i ){
		const ARP_RequestInfo& req = m_ARP_RequestQueue[i];
		if( ipaddr == req.dst_ipaddr ){
			is_found = true;
			break;
		}
	}

	return is_found;
}

ARP_Resolver::ARP_Resolver()
{}

ARP_Resolver& ARP_Resolver::Instance()
{
	static ARP_Resolver s_Instance;
	return s_Instance;
}


ARP_ResolveResult ARP_Resolver::Resolve( uint32_t src_ipaddr, uint32_t dst_ipaddr )
{
	ARP_ResolveResult result = m_ARP_Tbl.Resolve( dst_ipaddr );
	if( !result.is_resolved ){
		m_ARP_Request.Register( dst_ipaddr );
	}

	return result;
}

bool ARP_Resolver::Process_ARP_Packet( const PacketPtr& arppkt )
{
	bool result = true;
	ARP arphdr = arppkt->Get_ARP();

	switch( arphdr.OpCode() ){
	case k_ARP_Operation_ARP_Request:
		process_ARP_Request( arppkt );
		break;
	case k_ARP_Operation_ARP_Reply:
		m_ARP_Tbl.Register( arphdr.SrcIpAddr(), arphdr.SrcMacAddr() );
		m_ARP_Request.Remove( arphdr.SrcIpAddr() );
		break;
	default:
		result = false;
		break;
	}

	return result;
}

void ARP_Resolver::process_ARP_Request( const PacketPtr& arppkt )
{

	ARP arphdr = arppkt->Get_ARP();

	// 自分宛かどうかにせよ、相手のIPとMACアドレス対応をARPテーブルに登録
	m_ARP_Tbl.Register( arphdr.SrcIpAddr(), arphdr.SrcMacAddr() );

	// ARP Request のあて先IPアドレスが自分宛か調べる
	uint8_t interface_id = 0;
	uint32_t own_ipaddr = arphdr.SrcIpAddr();
	InternetLayer& l3 = InternetLayer::Instance();
	bool is_own_ipaddr = l3.GetInterface_ByIPAddr( own_ipaddr, &interface_id );
	// 自分宛じゃなければ、ARP Request は無視する
	if( !is_own_ipaddr ){
		return;
	}

	// 自分宛なので、ARP Reply を送信
	LinkLayer& l2 = LinkLayer::Instance();
	const uint8_t* src_macaddr = l2.GetMacAddr( interface_id );
	PacketPtr arp_reply = Create_ARP_Reply( src_macaddr, own_ipaddr, arphdr.SrcMacAddr(), arphdr.SrcIpAddr() );

	l2.Send( arppkt, interface_id, arphdr.SrcMacAddr(), k_EthType_ARP );
}

void ARP_Resolver::Update()
{
	// ARPリクエストテーブルを更新し、必要があればARP Requestパケットを送信する
	m_ARP_Request.Update();
}



PacketPtr Create_ARP_Request( const uint8_t* srcmac, uint32_t srcip, uint32_t dstip )
{
	uint8_t arp_dstmac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	PacketPtr packet = Create_ARP_Packet();

	if( packet.isNull() ){
		return packet;
	}

	ARP arp = packet->Get_ARP();
	arp.HwType( 0x0001 );			// Ethernet
	arp.Protocol( 0x0800 );			// TCP/IP
	arp.Hlen( 6 );					// MAC Address 長
	arp.Plen( 4 );					// IPv4 を表す
	arp.OpCode( 0x0001 );			// ARP Request
	arp.SrcMacAddr( srcmac );
	arp.DstMacAddr( arp_dstmac );
	arp.SrcIpAddr( srcip );
	arp.DstIpAddr( dstip );

	return packet;
}

PacketPtr Create_ARP_Reply( const uint8_t* src_macaddr,
							uint32_t src_ipaddr,
							const uint8_t* dst_macaddr,
							uint32_t dst_ipaddr
							)
{
	if( src_macaddr == nullptr || dst_macaddr == nullptr ){
		return PacketPtr();
	}

	PacketPtr packet = Create_ARP_Packet();
	if( packet.isNull() ){
		return packet;
	}

	ARP arp = packet->Get_ARP();
	arp.HwType( 0x0001 );			// Ethernet
	arp.Protocol( 0x0800 );			// TCP/IP
	arp.Hlen( 6 );					// MAC Address 長
	arp.Plen( 4 );					// IPv4 を表す
	arp.OpCode( 0x0002 );			// ARP Reply
	arp.SrcMacAddr( src_macaddr );
	arp.DstMacAddr( dst_macaddr );
	arp.SrcIpAddr( src_ipaddr );
	arp.DstIpAddr( dst_ipaddr );

	return packet;
}

