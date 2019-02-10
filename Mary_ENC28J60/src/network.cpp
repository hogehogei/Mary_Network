
#include "network.h"
#include "uart.h"
#include "drv/spi.h"
#include "drv/ethif_drv.hpp"
#include "lib/util/Endian.hpp"
#include "lib/net/link_layer.hpp"
#include "lib/net/internet_layer.hpp"

using ByteOrder = exlib::Endian<exlib::BigEndian>;

void Initialize_Network()
{
	// SPI0の初期化
	SPI_Drv::Settings spi_ch0_settings = {
			0,
			8,
			SPI_Drv::ROLE_MASTER,
			SPI_Drv::MODE0
	};
	if( SPI_Drv::Initialize( spi_ch0_settings ) ){
		UART_Print( "Init_SPI()" );
	}

	// ENC28J60 Eth Controller の初期化
	Eth_Settings eth0_settings = {
			0,
			{ 0x52, 0x54, 0x00, 0x12, 0xFF, 0x10 }
	};
	EthIf_Drv::Initialize( eth0_settings );

	// LinkLayer の初期化
	LinkLayer& l2 = LinkLayer::Instance();
	Eth_If eth0 = EthIf_Drv::Instance( 0 );
	l2.CreatePort( 0, eth0 );

	// InternetLayer の初期化
	InternetLayer& l3 = InternetLayer::Instance();
	uint8_t eth0_ipaddr[] = { 192, 168, 24, 50 };
	Route eth0_ip_setting = { ByteOrder::GetUint32( eth0_ipaddr ),		// IPv4 Address
							  0xFFFFFF00								// NetMask
	};

	l3.Bind( 0, eth0_ip_setting );
}
