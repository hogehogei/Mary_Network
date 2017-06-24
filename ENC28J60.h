#ifndef    ENC28J60_H_INCLUDED
#define    ENC28J60_H_INCLUDED

#include "stdint.h"

//
//  SPI Instruction Set
//
#define    _RCR(v) (0x00 | (0x1F & (v)))    // Read Control Register
#define    _RBM()  (0x3A)                   // Read Buffer Memory
#define    _WCR(v) (0x40 | (0x1F & (v)))    // Write Control Register
#define    _WBM()  (0x7A)                   // Write Buffer Memory
#define    _BFS(v) (0x80 | (0x1F & (v)))    // Bit Field Set
#define    _BFC(v) (0xA0 | (0x1F & (v)))    // Bit Field Clear
#define    _SRC()  (0xFF)                   // System Reset Command (Soft Reset)

//
// Control Register Map
//

//
// Bank0
//
#define    ERDPTL    (0x00)
#define    ERDPTH    (0x01)
#define    EWRPTL    (0x02)
#define    EWRPTH    (0x03)
#define    ETXSTL    (0x04)
#define    ETXSTH    (0x05)
#define    ETXNDL    (0x06)
#define    ETXNDH    (0x07)
#define    ERXSTL    (0x08)
#define    ERXSTH    (0x09)
#define    ERXNDL    (0x0A)
#define    ERXNDH    (0x0B)
#define    ERXRDPTL  (0x0C)
#define    ERXRDPTH  (0x0D)
#define    ERXWRPTL  (0x0E)
#define    ERXWRPTH  (0x0F)
#define    EDMASTL   (0x10)
#define    EDMASTH   (0x11)
#define    EDMANDL   (0x12)
#define    EDMANDH   (0x13)
#define    EDMADSTL  (0x14)
#define    EDMADSTH  (0x15)
#define    EDMACSL   (0x16)
#define    EDMACSH   (0x17)
// 0x18 - 0x19 none
// 0x1A reserved
#define    EIE       (0x1B)
#define    EIR       (0x1C)
#define    ESTAT     (0x1D)
#define    ECON2     (0x1E)
#define    ECON1     (0x1F)

//
// Bank1
//
#define    EHT0      (0x00 | 0x20)
#define    EHT1      (0x01 | 0x20)
#define    EHT2      (0x02 | 0x20)
#define    EHT3      (0x03 | 0x20)
#define    EHT4      (0x04 | 0x20)
#define    EHT5      (0x05 | 0x20)
#define    EHT6      (0x06 | 0x20)
#define    EHT7      (0x07 | 0x20)
#define    EPMM0     (0x08 | 0x20)
#define    EPMM1     (0x09 | 0x20)
#define    EPMM2     (0x0A | 0x20)
#define    EPMM3     (0x0B | 0x20)
#define    EPMM4     (0x0C | 0x20)
#define    EPMM5     (0x0D | 0x20)
#define    EPMM6     (0x0E | 0x20)
#define    EPMM7     (0x0F | 0x20)
#define    EPMCSL    (0x10 | 0x20)
#define    EPMCSH    (0x11 | 0x20)
// 0x12 - 0x13 none
#define    EPMOL     (0x14 | 0x20)
#define    EPMOH     (0x15 | 0x20)
// 0x16 - 0x017 reserved
#define    ERXFCON   (0x18 | 0x20)
#define    EPKTCNT   (0x19 | 0x20)
// 0x1A reserved
// 0x1B - 0x1F    Same Bank0


//
// Bank2
//
#define    MACON1    (0x00 | 0x40 | 0x80)
// 0x01 reserved
#define    MACON3    (0x02 | 0x40 | 0x80)
#define    MACON4    (0x03 | 0x40 | 0x80)
#define    MABBIPG   (0x04 | 0x40 | 0x80)
// 0x05 none
#define    MAIPGL    (0x06 | 0x40 | 0x80)
#define    MAIPGH    (0x07 | 0x40 | 0x80)
#define    MACLCON1  (0x08 | 0x40 | 0x80)
#define    MACLCON2  (0x09 | 0x40 | 0x80)
#define    MAMXFLL   (0x0A | 0x40 | 0x80)
#define    MAMXFLH   (0x0B | 0x40 | 0x80)
// 0x0C - 0x0E reserved
// 0x0F none
// 0x10 - 0x11 reserved
#define    MICMD     (0x12 | 0x40 | 0x80)
// 0x13 none
#define    MIREGADR  (0x14 | 0x40 | 0x80)
// 0x15 reserved
#define    MIWRL     (0x16 | 0x40 | 0x80)
#define    MIWRH     (0x17 | 0x40 | 0x80)
#define    MIRDL     (0x18 | 0x40 | 0x80)
#define    MIRDH     (0x19 | 0x40 | 0x80)
// 0x1A reserved
// 0x1B - 0x1F    Same Bank0


//
// Bank3
//
#define    MAADR5    (0x00 | 0x60 | 0x80)
#define    MAADR6    (0x01 | 0x60 | 0x80)
#define    MAADR3    (0x02 | 0x60 | 0x80)
#define    MAADR4    (0x03 | 0x60 | 0x80)
#define    MAADR1    (0x04 | 0x60 | 0x80)
#define    MAADR2    (0x05 | 0x60 | 0x80)
#define    EBSTSD    (0x06 | 0x60 | 0x80)
#define    EBSTCON   (0x07 | 0x60)
#define    EBSTCSL   (0x08 | 0x60)
#define    EBSTCSH   (0x09 | 0x60)
#define    MISTAT    (0x0A | 0x60 | 0x80)
// 0x0B - 0x11 none
#define    EREVID    (0x12 | 0x60)
// 0x13 - 0x14 none
#define    ECOCON    (0x15 | 0x60)
// 0x16 reserved
#define    EFLOCON   (0x17 | 0x60)
#define    EPAUSL    (0x18 | 0x60)
#define    EPAUSH    (0x19 | 0x60)
// 0x1A reserved
// 0x1B - 0x1F    Same Bank0

//
// PHY Registers
//
#define    PHCON1    (0x00)
#define    PHSTAT1   (0x01)
#define    PHID1     (0x02)
#define    PHID2     (0x03)
#define    PHCON2    (0x10)
#define    PHSTAT2   (0x11)
#define    PHIE      (0x12)
#define    PHIR      (0x13)
#define    PHLCON    (0x14)


// Register access utility
#define    GETBANK(v)           ((v) >> 5 & 0x03)
#define    IS_MAC_MII_REG(v)    ((v) & 0x80)

// Rx/Tx Buffer のサイズ設定 (0x0000 - 0x1FFF)
#define    RX_BUFFER_START    ((uint16_t)(0x0000))
#define    RX_BUFFER_END      ((uint16_t)(0x1FFF - 0x0600))
#define    TX_BUFFER_START    ((uint16_t)(0x1FFF - 0x05FF))
#define    TX_BUFFER_END      ((uint16_t)(0x1FFF))

// 受信/送信できる最大フレーム長
#define    MAX_FRAME_LENGTH   ((uint16_t)(1500))

// mac/mii 関連のレジスタにアクセスする場合のみ
// 通常のレジスタより CS のホールド時間がかかるので
// ループでウェイトさせる
// 通常　　　　10ns
// MAC/MII　210ns
// 36MHz動作で約500ns くらい？みておく
#define    MAC_MII_REGISTER_ACCESS_WAIT    (18)

// Mac Address  52:54:00:12:FF:10
// チップの中には書かれていないので、自分でMACアドレスを決める必要がある
#define    MACADDR1    (0x52)
#define    MACADDR2    (0x54)
#define    MACADDR3    (0x00)
#define    MACADDR4    (0x12)
#define    MACADDR5    (0xFF)
#define    MACADDR6    (0x10)

typedef struct _Packet {
	uint8_t* data;
	uint32_t len;
} Packet;

//
// Ethernet Controller ENC28J60 初期化
//
void Init_ENC28J60(void);

// linkup してるかどうか
// 1: linkup   0: linkdown
int Is_LinkUP_ENC28J60(void);

// MAC Address を取得する
const uint8_t* Get_MACAddr_ENC28J60(void);

typedef struct _Packet Packet;
// 送信/受信用バッファ取得
Packet* Use_TxPktBuf_ENC28J60(void);
void Free_TxPktBuf_ENC28J60( Packet* packet );
Packet* Use_RxPktBuf_ENC28J60(void);
void Free_RxPktBuf_ENC28J60( Packet* packet );

// Packet を送る
void SendPacket_ENC28J60( const Packet* packet_out );

// Packet を受信する
int RecvPacket_ENC28J60( Packet** packet_in );
int CopyPacketFromRecvBuffer_ENC28J60( Packet* packet_in );
int Get_RemainPacketCount(void);



enum InterruptFlag {
	INT_ENABLE = (1 << 7),
	PKTIF  = (1 << 6),
	//DMAIF  = (1 << 5),    // 使わない
	LINKIF = (1 << 4),
	//TXIF   = (1 << 3),    // 使わない
	TXERIF = (1 << 1),
	RXERIF = (1 << 0)
};
enum ECN28J60_Result {
	RECV_NOPKT = (1 << 0),
	RECV_VALIDPKT = (1 << 1),
	RECV_DROPPKT  = (1 << 2),
	RECV_CRCERR   = (1 << 3),
	INT_LINKCHANGE = (1 << 4),
	INT_TXERROR = (1 << 5),
	INT_RXERROR = (1 << 6),
	INT_RECVPKT = (1 << 7)
};

// interrupt 有効/無効
void EnableErrorInterrupt_ENC28J60(void);
void EnableRecvPktInterrupt_ENC28J60(void);
void DisableInterrupt_ENC28J60(void);

// 割り込み用コールバック
int InterruptCallback_ENC28J60(void);

// 初期化ルーチン中で使用
void Reset_ENC28J60(void);
void Init_PHY_ENC28J60(void);
void Init_PktBuffer_ENC28J60(void);
void Init_PktFilter_ENC28J60(void);
void Init_MAC_ENC28J60(void);
void Init_Interrupt_ENC28J60(void);
void Init_EthControl_ENC28J60(void);
void Show_Setting_ENC28J60(void);

#endif
