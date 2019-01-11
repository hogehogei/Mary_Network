/*
 * LinkLayer.hpp
 *
 *  Created on: 2018/12/07
 *      Author: hogehogei
 */

#ifndef LIB_NET_LINK_LAYER_HPP_
#define LIB_NET_LINK_LAYER_HPP_

#include "drv/ethif.hpp"


struct NetworkPort
{
	uint8_t		interface_id;
	uint8_t		macaddr[6];
	Eth_If		ethif;
};

class LinkLayer
{
private:

	static constexpr int sk_PortNum = 1;

public:

	static LinkLayer& Instance();
	LinkLayer( const LinkLayer& ) = delete;
	LinkLayer& operator=( const LinkLayer& ) = delete;

	bool CreatePort( uint8_t interface_id, Eth_If ethif );
	const uint8_t* GetMacAddr( uint8_t interface_id );

	bool Send( PacketPtr packet, uint8_t interface_id, const uint8_t* dst_macaddr, uint16_t type );
	void Recv_AllInterface();

private:

	LinkLayer();

	void RecvPort( uint8_t interface_id, Eth_If ethif );
	bool RxFilter_MacAddr( uint8_t interface_id, const PacketPtr& packet );
	void PacketPass_UpperLayer( uint8_t interface_id, const PacketPtr& packet );

	NetworkPort m_Port[sk_PortNum];
};



#endif /* LIB_NET_LINK_LAYER_HPP_ */
