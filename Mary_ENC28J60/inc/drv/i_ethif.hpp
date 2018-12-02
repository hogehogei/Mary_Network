/*
 * i_ethernet.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef DRV_I_ETH_IF_HPP_
#define DRV_I_ETH_IF_HPP_

#include <lib/net/packet.hpp>

class I_EthIf
{
public:

	I_EthIf() {}
	virtual ~I_EthIf() {}

	virtual bool Send( const PacketPtr& frame ) = 0;
	virtual bool Recv( PacketPtr* frame ) = 0;

	virtual uint32_t get_RxRemainPacketCount() = 0;
	virtual bool isLinkUp() = 0;
	virtual const uint8_t* getMacAddr() const = 0;

private:
};


#endif /* DRV_I_ETH_IF_HPP_ */
