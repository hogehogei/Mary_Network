/*
 * eth_frame.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef DRV_PACKET_HPP_
#define DRV_PACKET_HPP_

#include <cstdint>
#include "lib/util/RefCount.hpp"
#include "lib/util/IntrusivePtr.hpp"

class Packet : public exlib::RefCount
{
public:

	Packet();
	Packet( uint16_t datasize );
	~Packet() noexcept;

	const uint8_t* Head() const;
	uint8_t* Head();
	const uint8_t* Data() const;
	uint8_t* Data();

	uint16_t Size() const;
	uint16_t DataSize() const;

private:

	uint8_t* m_RawPacket;
	uint16_t m_Len;
};

typedef exlib::IntrusivePtr<Packet> PacketPtr;

PacketPtr Create_Packet( uint16_t datasize );


#endif /* DRV_PACKET_HPP_ */
