/*
 * internet_layer.cpp
 *
 *  Created on: 2018/12/07
 *      Author: hogehogei
 */

#include "lib/net/internet_layer.hpp"
#include "lib/net/link_layer.hpp"
#include "lib/net/arp_resolver.hpp"
#include "lib/net/icmp_client.hpp"


uint16_t InternetLayer::s_IP_PktID = 0;


RoutingTable::RoutingTable()
 : m_RoutingTbl_Last( 0 )
{}

RoutingTable::~RoutingTable()
{}

bool RoutingTable::AddRoute( const RoutingEntry& entry )
{
	if( m_RoutingTbl_Last >= k_RoutingTblSize ){
		return false;
	}

	m_Entry[m_RoutingTbl_Last] = entry;
	++m_RoutingTbl_Last;

	return true;
}

bool RoutingTable::SearchRoute( uint32_t dst_ipaddr, RoutingEntry* next_hop ) const
{
	bool result = false;
	uint8_t	metric = 255;

	// @TODO	Longest Match を実装する
	//			現在の実装では　LongestMatch になっていない
	for( uint32_t i = 0; i < m_RoutingTbl_Last; ++i ){
		const RoutingEntry entry = m_Entry[m_RoutingTbl_Last];
		if( (entry.ipaddr & entry.netmask) == (dst_ipaddr & entry.netmask) ){
			if( entry.metric < metric ){
				*next_hop = entry;
				metric = entry.metric;
				result = true;
			}
		}
	}

	return result;
}




InternetLayer::InternetLayer()
 : m_IPAddr_BindTblIdx( 0 )
{}

InternetLayer& InternetLayer::Instance()
{
	static InternetLayer s_Instance;

	return s_Instance;
}

bool InternetLayer::Bind( uint8_t interface_id, const Route& route )
{
	if( m_IPAddr_BindTblIdx >= k_IPAddr_BindTbl_Size ){
		return false;
	}

	IPAddr_BindEntry bind;
	bind.ipaddr			= route.ipaddr;
	bind.netmask		= route.netmask;
	bind.interface_id	= interface_id;
	m_IPAddr_BindTbl[m_IPAddr_BindTblIdx] = bind;
	++m_IPAddr_BindTblIdx;

	// ルーティングテーブルにルートを登録する
	RoutingEntry entry;
	entry.ipaddr	= route.ipaddr & route.netmask;
	entry.netmask	= route.netmask;
	entry.gw		= 0;
	entry.metric	= 1;
	entry.interface_id = interface_id;
	m_RoutingTbl.AddRoute( entry );

	return true;
}

bool InternetLayer::AddRoute( const RoutingEntry& entry )
{
	// ルーティングテーブルにルートを登録する
	return m_RoutingTbl.AddRoute( entry );
}

void InternetLayer::Send( PacketPtr packet, uint32_t dst_ipaddr )
{
	// 宛先ルートの検索
	RoutingEntry route;
	if( !m_RoutingTbl.SearchRoute( dst_ipaddr, &route ) ){
		// 宛先ルートがないので送れません
		return;
	}

	IPv4 ipv4 = packet->Get_IPv4();
	ipv4.Version( 4 );										// IPv4
	ipv4.HdrLen( 5 );										// 32bit*5 = 160bit = 20byte
	ipv4.Tos( 0x00 );										// Type of service
	ipv4.Id( ++s_IP_PktID );								// Packet ID
	ipv4.Flag( 0x02 );										// Flag フラグメントを許可しない
	ipv4.Offset( 0x00 );									// Offset
	ipv4.TTL( 128 );										// Time to Live
	ipv4.SrcAddr( route.ipaddr );							// 宛先アドレス
	ipv4.DstAddr( dst_ipaddr );								// 送信元アドレス

	m_SendQueue.Push( packet );
}

void InternetLayer::Recv( uint8_t interface_id, const PacketPtr& packet )
{
	// IP Filter
	if( !RxFilter_IPv4Addr( interface_id, packet ) ){
		// フィルタ処理ではじかれたのでパケットを捨てる
		// 以降の処理は実施しない
		return;
	}

	// とりあえず、TCP/UDPが実装できていないので
	// ICMP以外は捨てる方針でいく
	IPv4 ipv4 = packet->Get_IPv4();
	if( ipv4.Protocol() == 0x01 ){
		ICMP_Client& icmp_client = ICMP_Client::Instance();
		icmp_client.Recv( packet );
	}
}

bool InternetLayer::GetInterface_ByIPAddr( uint32_t ipaddr, uint8_t* interface_id ) const
{
	bool is_found = false;

	for( uint32_t i = 0; i < k_IPAddr_BindTbl_Size; ++i ){
		if( m_IPAddr_BindTbl[i].ipaddr == ipaddr ){
			*interface_id = m_IPAddr_BindTbl[i].interface_id;
			is_found = true;
			break;
		}
	}

	return is_found;
}

bool InternetLayer::GetInterface_ByDestRoute( uint32_t dst_ipaddr, uint8_t* interface_id ) const
{
	RoutingEntry entry;
	bool is_exist_route = m_RoutingTbl.SearchRoute( dst_ipaddr, &entry );

	if( is_exist_route ){
		*interface_id = entry.interface_id;
	}

	return is_exist_route;
}

bool InternetLayer::GetIPAddr_ByInterface( uint8_t interface_id, uint32_t* ipaddr ) const
{
	bool is_found = false;

	for( uint32_t i = 0; i < k_IPAddr_BindTbl_Size; ++i ){
		if( m_IPAddr_BindTbl[i].interface_id == interface_id ){
			*ipaddr = m_IPAddr_BindTbl[i].ipaddr;
			is_found = true;
			break;
		}
	}

	return is_found;
}

bool InternetLayer::GetNetMask_ByInterface( uint8_t interface_id, uint32_t* netmask ) const
{
	bool is_found = false;

	for( uint32_t i = 0; i < k_IPAddr_BindTbl_Size; ++i ){
		if( m_IPAddr_BindTbl[i].interface_id == interface_id ){
			*netmask = m_IPAddr_BindTbl[i].netmask;
			is_found = true;
			break;
		}
	}

	return is_found;
}

void InternetLayer::SendQueue_Process()
{
	int size = m_SendQueue.Size();
	for( int i = 0; i < size; ++i ){
		PacketPtr packet = m_SendQueue.Front();
		m_SendQueue.Pop();
		if( !TrySendPacket( packet ) ){
			m_SendQueue.Push( packet );
		}
	}
}

bool InternetLayer::TrySendPacket( PacketPtr packet )
{
	// ルート検索する
	RoutingEntry nexthop;
	IPv4 ipv4 = packet->Get_IPv4();
	bool is_nexthop_found = m_RoutingTbl.SearchRoute( ipv4.DstAddr(), &nexthop );

	if( !is_nexthop_found ){
		return false;
	}

	// ルート検索にて取得したGW宛に送信するため、ARP問い合わせを行う
	ARP_Resolver& arp_resolver = ARP_Resolver::Instance();
	ARP_ResolveResult result;
	uint32_t src_ipaddr = 0;
	GetIPAddr_ByInterface( nexthop.interface_id, &src_ipaddr );
	if( nexthop.gw == 0 ){
		// 宛先は自分のノードが接続されているネットワークである
		result = arp_resolver.Resolve( src_ipaddr, ipv4.DstAddr() );
	}
	else {
		// ゲートウェイのARP解決を試みる
		result = arp_resolver.Resolve( src_ipaddr, nexthop.ipaddr );
	}

	// ARP 問い合わせに成功すればそのまま送信する。
	// 失敗すれば送信待ちバッファにいれ待機する（ARPモジュールが自動で行う）
	if( result.is_resolved ){
		LinkLayer& l2 = LinkLayer::Instance();
		l2.Send( packet, nexthop.interface_id, result.hwaddr, k_EthType_IPv4 );
	}

	return result.is_resolved;
}

bool InternetLayer::RxFilter_IPv4Addr( uint8_t interface_id, const PacketPtr& packet )
{
	IPv4 ipv4hdr = packet->Get_IPv4();

	uint32_t pkt_ipaddr = ipv4hdr.DstAddr();
	uint32_t own_ipaddr = 0;
	uint32_t netmask = 0;
	if( !GetIPAddr_ByInterface( interface_id, &own_ipaddr ) ){
		return false;
	}
	if( !GetNetMask_ByInterface( interface_id, &netmask ) ){
		return false;
	}

	// リミテッド・ブロードキャストアドレスなので受信する
	if( pkt_ipaddr == 0xFFFFFFFF ){
		return true;
	}

	// 宛先が自身のIPだったら受け付ける
	if( pkt_ipaddr == own_ipaddr ){
		return true;
	}
	else {
		// 宛先がディレクテッド・ブロードキャストアドレスだったら受け付ける
		if( ((pkt_ipaddr & netmask) | netmask) == 0xFFFFFFFF ){
			return true;
		}
	}

	// それ以外は宛先IPアドレスが自分宛以外なので捨てる
	return false;
}
