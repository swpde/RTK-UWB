#ifndef _RINGBUFF_
#define _RINGBUFF_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdint.h>
#include "main.h"

struct ringbuff {
    void *buf;
    int16_t buff_size; //自定义环形容量 buf 最大 32KB

    uint16_t read_mirror: 1;
    uint16_t readPos: 15;
    uint16_t write_mirror: 1;
    uint16_t write_pos: 15;
    uint8_t residue: 8;
    uint8_t exist: 8;


};

//typedef struct
//{
//    uint8_t start_flag; // 起始标志(0xAA)
//    uint8_t ctrl_flags; // 控制位(见下方说明)
//    uint8_t packet_id; // 包序列号(0-65535循环)
//    uint8_t data_len;  // 数据长度(最大194)
//    uint8_t data[200];  // 数据载荷
////    uint16_t crc16;     // CRC16校验
//} SerialPacket;
typedef struct ringbuff *ringbuff_t;

#define RINGBUFF_EMPTY 0
#define RINGBUFF_FULL 1
#define RINGBUFF_HALFFULL 2

#define FLAG_SYN (1 << 0)        // 建立连接
#define FLAG_FRAG_START (1 << 1) // 分片开始
#define FLAG_FRAG_END (1 << 2)   // 分片结束
#define FLAG_FIN (1 << 3)        // 结束连接

static inline int32_t ringbuff_state_get(ringbuff_t p_rb) {
    if (p_rb->readPos == p_rb->write_pos) {
        if (p_rb->read_mirror == p_rb->write_mirror)
            return RINGBUFF_EMPTY;
        else
            return RINGBUFF_FULL;
    } else
        return RINGBUFF_HALFFULL;
}
//剩余的长度
//#define ringbuff_data_space_len(p_rb) ((p_rb)->buff_size - ringbuff_data_len(p_rb))

//数据的长度
int16_t ringbuff_data_len(ringbuff_t p_rb);

int32_t ringbuff_getchar(ringbuff_t p_rb, uint8_t *ch);

int32_t ringbuff_putchar(ringbuff_t p_rb, const uint8_t ch);

uint16_t ringbuff_data_space_len(ringbuff_t p_rb);

int8_t ringbuff_getdata(ringbuff_t p_rb, uint8_t *data, uint16_t length);

int8_t ringbuff_putdata(ringbuff_t p_rb, uint8_t *data, uint16_t length);

uint16_t ringbuff_getdata_all(ringbuff_t p_rb, uint8_t *data);

uint8_t Command_Send_Data(uint8_t *send_data,  uint16_t total_len,uint16_t chunk_len) ;
uint8_t Command_Send_Data_t(uint8_t *send_data, uint8_t packets_num, uint16_t total_len) ;
uint8_t Command_Analysis_Data(uint8_t *receive_data, uint16_t total_len,uint16_t Receive_last_length) ;



void ringbuff_debug(ringbuff_t p_rb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RINGBUFF_ */