#include "network.h"
#include "ENC28J60.h"
#include "uart.h"

static uint16_t sPacketID = 0;

static uint16_t Get_IP_PktID(void)
{
	return sPacketID++;
}

uint16_t Get_BEU16( const uint8_t* p )
{
	return (p[0] << 8) | p[1];
}

uint32_t Get_BEU32( const uint8_t* p )
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

void Store_BEU16( uint8_t* dst, uint16_t data )
{
	dst[0] = data >> 8;
	dst[1] = data & 0xFF;
}

void Store_BEU32( uint8_t* dst, uint32_t data )
{
	dst[0] = data >> 24;
	dst[1] = (data >> 16) & 0xFF;
	dst[2] = (data >> 8) & 0xFF;
	dst[3] = data & 0xFF;
}

uint16_t Checksum( const uint8_t* data, int len )
{
	int remain_size = len;
	const uint8_t* p = data;

	uint32_t sum = 0;
	while( remain_size >  1 ){
		sum += Get_BEU16(p);
		p += 2;
		remain_size -= 2;
	}
	// 残り1バイト余ってる？
	if( remain_size ){
		sum += *p;
	}

	sum = (sum & 0xFFFF) + (sum >> 16);
	sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

uint16_t IP_Checksum( const IP_Hdr* iphdr )
{
	return Checksum( (const uint8_t*)iphdr, sizeof(IP_Hdr) );
}

uint16_t ICMP_Checksum( const ICMP_Hdr* icmphdr, uint32_t datalen )
{
	return Checksum( (const uint8_t*)icmphdr, sizeof(ICMP_Hdr) + datalen );
}

void Set_EtherHdr( Ether_Hdr* ethhdr, const uint8_t* macdst )
{
	// MAC Address 設定
	int i = 0;
	const uint8_t* macsrc = Get_MACAddr_ENC28J60();
	for( i = 0; i < 6; ++i ){
		ethhdr->macdst[i] = macdst[i];
		ethhdr->macsrc[i] = macsrc[i];
	}
}

void Set_ICMP_IPHdr( IP_Hdr* iphdr )
{
	iphdr->version_hdrlen  = (4 << 4) | 5;                 // IPv4, 32bit*5 = 160bit = 20byte
	iphdr->tos             = 0x00;                         // Type of service
	Store_BEU16( iphdr->id, Get_IP_PktID() );
	Store_BEU16( iphdr->flag_offset, 0x02 << 13 | 0x00 );  // パケットのフラグメントを許可しない
	iphdr->ttl         = 128;                              // Time to Live
	iphdr->protocol    = 0x01;                             // ICMP
	Store_BEU16( iphdr->chksum, 0x0000 );
	Store_BEU16( iphdr->chksum, IP_Checksum( iphdr ) );
}

Packet* Create_ICMPEchoRequest( uint32_t src_ip, uint32_t dst_ip )
{
	Packet* pktbuf = Use_TxPktBuf_ENC28J60();
	uint8_t* buf_head = pktbuf->data;
	uint32_t icmp_datalen = 4 + 32;    // 4byte

	// Ethernet Header の作成
	Ether_Hdr* ethhdr = (Ether_Hdr*)buf_head;
	// 宛先MACは決め打ちにしちゃう！
	uint8_t dstmac[6] = { 0x90, 0x1B, 0x0E, 0x16, 0x47, 0x89 };
	Set_EtherHdr( ethhdr,  dstmac );
	Store_BEU16( ethhdr->type, 0x0800 );  // type: IP

	// IP Header の作成
	IP_Hdr* iphdr = (IP_Hdr*)(buf_head + sizeof(Ether_Hdr));
	// src/dst IPアドレスとデータ長を入れてから渡す
	Store_BEU16( iphdr->total_len, sizeof(IP_Hdr) + sizeof(ICMP_Hdr) + icmp_datalen );
	Store_BEU32( iphdr->src_ip, src_ip );
	Store_BEU32( iphdr->dst_ip, dst_ip );
	Set_ICMP_IPHdr( iphdr );

	// ICMP Header の作成
	ICMP_Hdr* icmphdr = (ICMP_Hdr*)(buf_head + sizeof(Ether_Hdr) + sizeof(IP_Hdr));
	icmphdr->type = 0x08;      // ICMP Echo Request
	icmphdr->code = 0x00;      // Code
	Store_BEU16( icmphdr->data, 0x0100 );    // ICMP id
	Store_BEU16( icmphdr->data+2, 0x0003 );  // ICMP sequence number
	int i = 0;
	// ダミーデータ作成  Windowsっぽい感じで
	for( i = 0; i < 32; ++i ){
		icmphdr->data[4+i] = 0x61 + (i % 0x17);
	}
	Store_BEU16( icmphdr->chksum, 0x0000 );  // checksum 0初期化してから計算
	Store_BEU16( icmphdr->chksum, ICMP_Checksum( icmphdr, icmp_datalen ) );

	// パケットサイズを入れて返す
	pktbuf->len = sizeof(Ether_Hdr) + sizeof(IP_Hdr) + sizeof(ICMP_Hdr) + icmp_datalen;
	return pktbuf;
}
