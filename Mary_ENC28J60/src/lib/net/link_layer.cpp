/*
 * link_layer.cpp
 *
 *  Created on: 2019/01/06
 *      Author: hogehogei
 */

#include "lib/net/link_layer.hpp"
#include "lib/net/internet_layer.hpp"
#include "lib/net/arp_resolver.hpp"

LinkLayer::LinkLayer()
{}

LinkLayer& LinkLayer::Instance()
{
	static LinkLayer s_LinkLayer;
	return s_LinkLayer;
}

bool LinkLayer::CreatePort( uint8_t interface_id, Eth_If ethif )
{
	if( interface_id >= sk_PortNum ){
		return false;
	}

	NetworkPort port;
	port.interface_id = interface_id;

	const uint8_t* src = ethif.getMacAddr();
	for( int i = 0; i < 6; ++i ){
		port.macaddr[i] = src[i];
	}
	port.ethif = ethif;

	m_Port[interface_id] = port;

	return true;
}

const uint8_t* LinkLayer::GetMacAddr( uint8_t interface_id )
{
	return m_Port[interface_id].ethif.getMacAddr();
}

bool LinkLayer::Send( PacketPtr packet, uint8_t interface_id, const uint8_t* dst_macaddr, uint16_t type )
{
	Eth_If ethif = m_Port[interface_id].ethif;
	Ethernet ethhdr = packet->Get_Eth();

	ethhdr.SrcMacAddr( ethif.getMacAddr() );
	ethhdr.DstMacAddr( dst_macaddr );
	ethhdr.Type( type );

	return ethif.Send( packet );
}

void LinkLayer::Recv_AllInterface()
{
	for( int i = 0; i < sk_PortNum; ++i ){
		RecvPort( i, m_Port[i].ethif );
	}
}

void LinkLayer::RecvPort( uint8_t interface_id, Eth_If ethif )
{
	uint32_t rxnum = ethif.get_RxRemainPacketCount();
	PacketPtr packet;

	for( uint32_t rxcnt = 0; rxcnt < rxnum; ++rxcnt ){
		if( ethif.Recv( &packet ) ){
			if( RxFilter_MacAddr( interface_id, packet ) ){
				// 受信OK。上位モジュールに渡す
				PacketPass_UpperLayer( interface_id, packet );
			}
		}
	}
}

bool LinkLayer::RxFilter_MacAddr( uint8_t interface_id, const PacketPtr& packet )
{
	Eth_If ethif = m_Port[interface_id].ethif;
	Ethernet ethhdr = packet->Get_Eth();

	if( IsSameMacAddr( ethif.getMacAddr(), ethhdr.DstMacAddr() ) ||
		IsBroadCastMacAddr( ethhdr.DstMacAddr() )
	){
		// MAC Address は有効なので受け付ける
		return true;
	}

	return false;
}

void LinkLayer::PacketPass_UpperLayer( uint8_t interface_id, const PacketPtr& packet )
{
	Ethernet ethhdr = packet->Get_Eth();

	if( ethhdr.Type() == k_EthType_IPv4 ){
		InternetLayer& l3 = InternetLayer::Instance();
		l3.Recv( interface_id, packet );
	}
	if( ethhdr.Type() == k_EthType_ARP ){
		ARP_Resolver& arp_resolver = ARP_Resolver::Instance();
		arp_resolver.Process_ARP_Packet( packet );
	}
}
