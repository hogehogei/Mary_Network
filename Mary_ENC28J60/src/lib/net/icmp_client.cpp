/*
 * icmp_client.cpp
 *
 *  Created on: 2019/01/13
 *      Author: hogehogei
 */

#include "lib/net/icmp_client.hpp"
#include "lib/net/internet_layer.hpp"
#include "lib/util/endian.hpp"
#include "uart.h"

using ByteOrder = exlib::Endian<exlib::BigEndian>;

uint16_t ICMP_Client::s_ICMPEchoSeqID = 0;

ICMP_Client::ICMP_Client()
{}

ICMP_Client& ICMP_Client::Instance()
{
	static ICMP_Client s_Instance;

	return s_Instance;
}

void ICMP_Client::Recv( const PacketPtr& packet )
{
	// ICMP Header の解析
	ICMP icmp = packet->Get_ICMP();

	if( icmp.Type() == 0x00 ){
		// ICMP Echo Reply
		UART_Print( "Get ICMP Reply" );
	}
}

void ICMP_Client::Ping( uint32_t dst_ipaddr )
{
	uint16_t icmp_payload_len = 32;
	PacketPtr packet = Create_ICMP_Packet( icmp_payload_len );

	IPv4 ipv4 = packet->Get_IPv4();
	ipv4.Protocol( 0x01 );						// ICMP

	// ICMP Header の作成
	ICMP icmp = packet->Get_ICMP();

	icmp.Type( 0x08 );							// ICMP Echo Request
	icmp.Code( 0x00 );							// Code

	uint8_t* data = icmp.Data();
	ByteOrder::SetUint16( data, 0x0100 );				// ICMP id
	ByteOrder::SetUint16( data+2, ++s_ICMPEchoSeqID );	// ICMP sequence number

	int i = 0;
	// ダミーデータ作成  Windowsっぽい感じで
	for( i = 0; i < 32; ++i ){
		data[4+i] = 0x61 + (i % 0x17);
	}
	icmp.CalculateChkSum();

	// 送信
	InternetLayer& l3 = InternetLayer::Instance();
	l3.Send( packet, dst_ipaddr );
}

