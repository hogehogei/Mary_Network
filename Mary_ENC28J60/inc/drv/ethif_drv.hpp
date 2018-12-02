/*
 * ethernet_drv.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef DRV_ETHIF_DRV_HPP_
#define DRV_ETHIF_DRV_HPP_

#include <drv/ethif.hpp>
#include "lib/net/eth_settings.hpp"
#include "ENC28J60.h"

class EthIf_Drv
{
public:

	static bool Initialize( const Eth_Settings& settings );
	static Eth_If& Instance( uint8_t interface_id );

	static void ENC28J60_ch1_Callback();

private:

	EthIf_Drv();
	~EthIf_Drv();
	EthIf_Drv( const EthIf_Drv& drv );
	EthIf_Drv& operator=( const EthIf_Drv& drv );

	static constexpr uint32_t k_EthIf_Num = 1;				//! このマイコンで管理するEthernetインターフェース数
	static constexpr uint8_t  k_ENC28J60_SPI_Ch = 0;		//! ENC28J60が使用するSPIチャンネル

	static ENC28J60 m_ENC28J60;
	static Eth_If m_Eth_If[k_EthIf_Num];
	static Eth_If m_Invalid_If;								//! ダミーのEthernetモジュール、戻り値が不正の場合に使用する
};


#endif /* DRV_ETHIF_DRV_HPP_ */
