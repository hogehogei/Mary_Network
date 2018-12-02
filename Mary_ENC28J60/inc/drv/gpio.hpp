/*
 * gpio.hpp
 *
 *  Created on: 2018/11/18
 *      Author: hogehogei
 */

#ifndef DRV_GPIO_HPP_
#define DRV_GPIO_HPP_

#include <cstdint>

/**
 * @brief	GPIO 操作クラス
 * 			GPIO の操作を簡単化するクラス。
 */
class GPIO
{
public:
	enum PortLogic {
		PORT_LOGIC_H_ON,
		PORT_LOGIC_L_ON
	};

public:

	//! 空のコンストラクタ
	GPIO();
	/**
	 * @brief	コンストラクタ
	 * @param	[in]	port	ポートアドレス
	 * @param	[in]	bitpos	ビット位置
	 * @param	[in]	logic	ポート論理( HiでONか、LoでONかということ )
	 */
	GPIO( volatile uint32_t* port, uint8_t bitpos, PortLogic logic );
	//! デストラクタ
	~GPIO();


	/**
	 * @brief	ポートON
	 * 			コンストラクタで指定したポート論理に従う
	 */
	void On();
	/**
	 * @brief	ポートOFF
	 * 			コンストラクタで指定したポート論理に従う
	 */
	void Off();
	/**
	 * @brief	ポートHi
	 */
	void Hi();
	/**
	 * @brief	ポートLo
	 */
	void Lo();
	/**
	 * @brief	ポート論理セット
	 * @param	[in] on_off		ポート論理 trueでON、falseでOFF
	 */
	void Set( bool on_off );
	/**
	 * @brief	ポート論理反転
	 */
	void Invert();

	/**
	 * @brief	ON論理かどうか返す
	 */
	bool isOn() const;
	/**
	 * @brief	OFF論理かどうか返す
	 */
	bool isOff() const;
	/**
	 * @brief	ポート論理がHiかどうか返す
	 */
	bool isHi() const;
	/**
	 * @brief	ポート論理がLoかどうか返す
	 */
	bool isLo() const;

private:

	volatile uint32_t*	mGPIO_Port;				//! ポートアドレス
	uint32_t			mBitPos;				//! ビット位置
	PortLogic 			mPortLogic;				//! ポート論理
};


#endif /* DRV_GPIO_HPP_ */
