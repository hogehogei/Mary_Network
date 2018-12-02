#ifndef    NETWORK_H_INCLUDED
#define    NETWORK_H_INCLUDED

#include "stdint.h"


typedef struct _ARP_Hdr {
	uint8_t hw_type[2];
	uint8_t protocol[2];
	uint8_t hlen;
	uint8_t plen;
	uint8_t op_code[2];
	uint8_t macsrc[6];
	uint8_t ipsrc[4];
	uint8_t macdst[6];
	uint8_t ipdst[4];
} ARP_Hdr;

typedef struct _IP_Hdr {
	uint8_t version_hdrlen;
	uint8_t tos;
	uint8_t total_len[2];
	uint8_t id[2];
	uint8_t flag_offset[2];    // flag 3bit, fragment offset, 13bit
	uint8_t ttl;
	uint8_t protocol;
	uint8_t chksum[2];
	uint8_t src_addr[4];
	uint8_t dst_addr[4];
	// option, padding はとりあえず無視！
	uint8_t data[0];
} IP_Hdr;

typedef struct _ICMP_Hdr {
	uint8_t type;
	uint8_t code;
	uint8_t chksum[2];
	uint8_t data[0];
} ICMP_Hdr;

typedef struct _Host {
	uint8_t macaddr[6];
	uint8_t ipaddr[4];
	uint8_t netmask;    // CIDR で指定
} Host;

uint16_t Get_BEU16( const uint8_t* p );
uint32_t Get_BEU32( const uint8_t* p );
void Store_BEU16( uint8_t* dst, uint16_t data );
void Store_BEU32( uint8_t* dst, uint32_t data );

uint16_t Checksum( const uint8_t* data, int len );
void IP_StoreChecksum( IP_Hdr* iphdr );
void ICMP_StoreChecksum( ICMP_Hdr* icmphdr, uint32_t datalen );

int IsSameMacAddr( const uint8_t* mac_a, const uint8_t* mac_b );
int IsBroadCastMacAddr( const uint8_t* macaddr );

void Set_EtherHdr( Ether_Hdr* ethhdr, const uint8_t* macsrc, const uint8_t* macdst );
void Set_Default_IPHdr( IP_Hdr* iphdr );
ICMP_Hdr* Create_ICMPHdr( uint8_t* frame_head, const Host* src, const Host* dst, uint32_t datalen );

typedef struct _Packet Packet;
// ICMP Echo Request のパケット作成
Packet* Create_ICMPEchoRequest( const Host* src, const Host* dst );
// ARP Request 作成
Packet* Create_ARPRequest( const Host* src, const Host* dst );

int IsARPReply( const Packet* pkt );
int IsICMPEchoReply( const Packet* pkt );
void Process_ARPReply( const Packet* pkt, Host* dst );

void Show_EtherHdr( const Ether_Hdr* hdr );

#endif
