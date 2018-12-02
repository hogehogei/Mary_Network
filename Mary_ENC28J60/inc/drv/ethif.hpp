/*
 * ethernet.hpp
 *
 *  Created on: 2018/11/24
 *      Author: hogehogei
 */

#ifndef DRV_ETHIF_HPP_
#define DRV_ETHIF_HPP_

#include <lib/net/packet.hpp>
#include <cstdint>


class I_EthIf;
class Eth_If
{
public:

	Eth_If();
	~Eth_If();

	bool Send( const PacketPtr& frame );
	bool Recv( PacketPtr* frame );

	uint32_t get_RxRemainPacketCount();
	bool isLinkUp();
	const uint8_t* getMacAddr() const;

private:

	friend class EthIf_Drv;
	Eth_If( I_EthIf* impl );

	I_EthIf* m_Impl;										//! pImpl 実装はこっち
};





#endif /* DRV_ETHIF_HPP_ */
