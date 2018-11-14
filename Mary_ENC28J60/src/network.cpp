#include "network.h"
#include "ENC28J60.h"
#include "uart.h"

static uint16_t sPacketID = 0;
static uint16_t sICMPEchoSeqID = 0;

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

void IP_StoreChecksum( IP_Hdr* iphdr )
{
	Store_BEU16( iphdr->chksum, 0x0000 );  // checksum 0初期化してから計算
	uint16_t chksum = Checksum( (const uint8_t*)iphdr, sizeof(IP_Hdr) );
	Store_BEU16( iphdr->chksum, chksum );
}

void ICMP_StoreChecksum( ICMP_Hdr* icmphdr, uint32_t datalen )
{
	Store_BEU16( icmphdr->chksum, 0x0000 );  // checksum 0初期化してから計算
	uint16_t chksum = Checksum( (const uint8_t*)icmphdr, sizeof(ICMP_Hdr) + datalen );
	Store_BEU16( icmphdr->chksum, chksum );
}

int IsSameMacAddr( const uint8_t* mac_a, const uint8_t* mac_b )
{
	int result = 1;
	int i = 0;
	for( i = 0; i < 6; ++i ){
		if( mac_a[i] != mac_b[i] ){
			result = 0;
		}
	}

	return result;
}

int IsBroadCastMacAddr( const uint8_t* macaddr )
{
	int result = 1;
	int i = 0;
	for( i = 0; i < 6; ++i ){
		if( macaddr[i] != 0xFF ){
			result = 0;
		}
	}

	return result;
}

void Set_EtherHdr( Ether_Hdr* ethhdr, const uint8_t* macsrc, const uint8_t* macdst )
{
	// MAC Address 設定
	int i = 0;
	for( i = 0; i < 6; ++i ){
		ethhdr->macdst[i] = macdst[i];
		ethhdr->macsrc[i] = macsrc[i];
	}
}

void Set_Default_IPHdr( IP_Hdr* iphdr )
{
	iphdr->version_hdrlen  = (4 << 4) | 5;                 // IPv4, 32bit*5 = 160bit = 20byte
	iphdr->tos             = 0x00;                         // Type of service
	Store_BEU16( iphdr->id, Get_IP_PktID() );
	Store_BEU16( iphdr->flag_offset, 0x02 << 13 | 0x00 );  // パケットのフラグメントを許可しない
	iphdr->ttl         = 128;                              // Time to Live
}

ICMP_Hdr* Create_ICMPHdr( uint8_t* frame_head, const Host* src, const Host* dst, uint32_t datalen )
{
	// Ethernet Header の作成
	Ether_Hdr* ethhdr = (Ether_Hdr*)frame_head;
	Set_EtherHdr( ethhdr, src->macaddr, dst->macaddr );
	Store_BEU16( ethhdr->type, 0x0800 );  // type: IP

	// IP Header の作成
	IP_Hdr* iphdr = (IP_Hdr*)(ethhdr->data);
	Set_Default_IPHdr( iphdr );
	Store_BEU16( iphdr->total_len, sizeof(IP_Hdr) + sizeof(ICMP_Hdr) + datalen );
	int i = 0;
	for( i = 0; i < 4; ++i ){
		iphdr->src_addr[i] = src->ipaddr[i];
		iphdr->dst_addr[i] = dst->ipaddr[i];
	}
	iphdr->protocol    = 0x01;    // ICMP
	IP_StoreChecksum( iphdr );

	return reinterpret_cast<ICMP_Hdr*>(iphdr->data);
}

Packet* Create_ICMPEchoRequest( const Host* src, const Host* dst )
{
	Packet* pktbuf = Use_TxPktBuf_ENC28J60();

	uint16_t icmp_data_len = 32;  // ICMPデータは32byte
	uint16_t icmp_field_len = icmp_data_len + sizeof(ICMP_Hdr);

	// ICMP Header の作成
	ICMP_Hdr* icmphdr = Create_ICMPHdr( pktbuf->data, src, dst,  icmp_data_len );
	icmphdr->type = 0x08;      // ICMP Echo Request
	icmphdr->code = 0x00;      // Code
	Store_BEU16( icmphdr->data, 0x0100 );          // ICMP id
	Store_BEU16( icmphdr->data+2, ++sICMPEchoSeqID );  // ICMP sequence number

	int i = 0;
	// ダミーデータ作成  Windowsっぽい感じで
	for( i = 0; i < 32; ++i ){
		icmphdr->data[4+i] = 0x61 + (i % 0x17);
	}
	ICMP_StoreChecksum( icmphdr, icmp_data_len );

	// パケットサイズを入れて返す
	pktbuf->len = sizeof(Ether_Hdr) + sizeof(IP_Hdr) + icmp_field_len;
	return pktbuf;
}

Packet* Create_ARPRequest( const Host* src, const Host* dst )
{
	// MACアドレスはブロードキャスト
	uint8_t dstmac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	Packet* pkt = Use_TxPktBuf_ENC28J60();
	Ether_Hdr* ethhdr = (Ether_Hdr*)pkt->data;
	Set_EtherHdr( ethhdr, src->macaddr, dstmac );
	Store_BEU16( ethhdr->type, 0x0806 );    // type: ARP

	ARP_Hdr* arphdr = (ARP_Hdr*)ethhdr->data;
	Store_BEU16( arphdr->hw_type, 0x0001 );    // Ethernet
	Store_BEU16( arphdr->protocol, 0x0800 );   // TCP/IP
	arphdr->hlen = 6;    // macaddress の長さ
	arphdr->plen = 4;    // ipv4
	Store_BEU16( arphdr->op_code, 0x0001 );   // ARP Request
	int i = 0;
	for( i = 0; i < arphdr->hlen; ++i ){
		arphdr->macsrc[i] = src->macaddr[i];
		arphdr->macdst[i] = 0x00;    // 送信先のMACアドレスはわからん
	}
	for( i = 0; i < arphdr->plen; ++i ){
		arphdr->ipsrc[i] = src->ipaddr[i];
		arphdr->ipdst[i] = dst->ipaddr[i];
	}

	pkt->len = sizeof(Ether_Hdr) + sizeof(ARP_Hdr);
	return pkt;
}

int IsARPReply( const Packet* pkt )
{
	const Ether_Hdr* ethhdr = (const Ether_Hdr*)pkt->data;
	uint16_t type = Get_BEU16( ethhdr->type );
	if( type != 0x0806 ){    // ethernet header の type がARPじゃない
		return 0;
	}

	const ARP_Hdr* arphdr = (const ARP_Hdr*)ethhdr->data;
	uint16_t hw_type = Get_BEU16( arphdr->hw_type );
	uint16_t protocol = Get_BEU16( arphdr->protocol );
	uint16_t op_code = Get_BEU16( arphdr->op_code );

	if( hw_type == 0x0001 && protocol == 0x0800 && op_code == 2 ){
		return 1;
	}

	return 0;
}

void Process_ARPReply( const Packet* pkt, Host* dst )
{
	const ARP_Hdr* arphdr = (const ARP_Hdr*)(pkt->data + sizeof(Ether_Hdr));
	int i = 0;
	for( i = 0; i < 6; ++i ){
		// ARP応答の送信元MAC Addressが知りたい
		dst->macaddr[i] = arphdr->macsrc[i];
	}
}

int IsICMPEchoReply( const Packet* pkt )
{
	const Ether_Hdr* ethhdr = (const Ether_Hdr*)pkt->data;
	uint16_t type = Get_BEU16( ethhdr->type );
	if( type != 0x0800 ){    // ethernet header の type がIPじゃない
		return 0;
	}

	const IP_Hdr* iphdr = (const IP_Hdr*)ethhdr->data;
	if( iphdr->protocol != 0x01 ){
		return 0;           // IP header の protocol がICMPじゃない
	}

	const ICMP_Hdr* icmphdr = (const ICMP_Hdr*)iphdr->data;
	if( icmphdr->type != 0x00 ){  // ICMPの type が ICMP Echo Reply じゃない
		return 0;
	}

	return 1;
}

void Show_EtherHdr( const Ether_Hdr* hdr )
{
	UART_Print( "Ethernet Header" );
	UART_Print( "destination macaddr" );
	UART_HexPrint( hdr->macdst, 6 ); UART_NewLine();
	UART_Print( "source macaddr" );
	UART_HexPrint( hdr->macsrc, 6 ); UART_NewLine();
	UART_Print( "type" );
	UART_HexPrint( hdr->type, 2 ); UART_NewLine();
}
