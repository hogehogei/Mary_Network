/*
 * icmp_client.hpp
 *
 *  Created on: 2019/01/06
 *      Author: hogehogei
 */

#ifndef LIB_NET_ICMP_CLIENT_HPP_
#define LIB_NET_ICMP_CLIENT_HPP_


class ICMP_Client
{
public:

	static ICMP_Client& Instance();
	ICMP_Client( const ICMP_Client& ) = delete;
	ICMP_Client& operator=( const ICMP_Client& ) = delete;

	void Ping( uint32_t dst_ipaddr );

private:
};

void Recv_ICMP_Reply();


#endif /* LIB_NET_ICMP_CLIENT_HPP_ */
