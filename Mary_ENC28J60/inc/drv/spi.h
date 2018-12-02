
#ifndef  SPI_H
#define  SPI_H

#include "stdint.h"

class SPI;

/**
 * @brief	SPIドライバクラス
 * 			SPIの初期化、SPIチャンネルの管理を担当する。
 * 			マイコンによるハードウェアの違いや、SPIチャンネル数の違いを隠ぺいするのが意図。
 */
class SPI_Drv
{
public:
	enum Role
	{
		ROLE_MASTER,				//! SPI Master
		ROLE_SLAVE					//! SPI Slave
	};

	enum Mode
	{
		MODE0,						//! SPI MODE0
		MODE1,						//! SPI MODE1
		MODE2,						//! SPI MODE2
		MODE3						//! SPI MODE3
	};

	//! 初期化オプション
	struct Settings
	{
		uint8_t channel;			//!	設定対象チャンネル
		uint8_t bitlen;				//! ビット長
		Role	role;				//! Master/Slave
		Mode	mode;				//! SPIモード (0～3)
	};

public:

	/**
	 * @brief	SPI初期化
	 * @param	[in]	settings	SPI初期化設定
	 */
	static bool Initialize( const SPI_Drv::Settings& settings );
	/**
	 * @brief	インスタンス取得
	 * 			Initialize() で対象チャンネルを取得し、Instance() で取得したハンドルに対して操作する。
	 * @param	[in]	channel		操作するチャンネル
	 */
	static SPI& Instance( uint8_t channel );

private:

	static constexpr uint8_t k_SPI_Channels = 1;		//! このマイコンで使用するSPIチャンネル数
														//! ハードウェアの実装によって変えること

	static SPI m_SPI[k_SPI_Channels];					//! SPIのハンドル
														//! 実際の操作はこちらへ実装する
};

/**
 * @brief	SPI操作クラス
 * 			SPIによるデータの送受信について、マイコンによるハードウェアの違いを隠蔽する。
 */
class SPI
{
public:

	SPI();
	~SPI();

	/**
	 * @brief	1データ送受信
	 * 			初期化時に設定したデータ長[bit]を1データとして送信する。
	 * @param	[in]	txdata		送信データ
	 * 								データ長以外のデータは反映しない。
	 */
	uint16_t TxRx( uint16_t txdata );
	/**
	 * @brief	データ送信
	 * @param	[in]	data		送信データ配列
	 * @param	[in]	datalen		送信データ長
	 */
	uint32_t Send( const uint16_t* data, uint32_t datalen );
	/**
	 * @brief	データ受信
	 * @param	[out]	dst			受信データ格納先
	 * @param	[in]	dstlen		受信データ格納先の配列長
	 */
	uint32_t Recv( uint16_t* dst, uint32_t dstlen );

private:

	friend class SPI_Drv;
	SPI( bool is_initailzed );

	bool m_Is_Initialized;								//! 初期化されたか表す
};

#endif
