/*
 * icmp.hpp
 *
 *  Created on: 2018/12/03
 *      Author: hogehogei
 */

#ifndef LIB_NET_ICMP_HPP_
#define LIB_NET_ICMP_HPP_

#include <cstdint>

struct ICMP_Hdr {
	uint8_t type;
	uint8_t code;
	uint8_t chksum[2];
	uint8_t data[0];
};


class ICMP
{
public:

	ICMP( uint8_t* icmphdr, uint16_t datalen );

	uint8_t Type() const;
	void Type( uint8_t type );

	uint8_t Code() const;
	void Code( uint8_t code );

	uint16_t ChkSum() const;
	void CalculateChkSum();

	const uint8_t* Data() const;
	uint8_t* Data();

private:

	ICMP_Hdr* m_ICMP_Hdr;
	uint16_t  m_DataLen;
};

#endif /* LIB_NET_ICMP_HPP_ */
