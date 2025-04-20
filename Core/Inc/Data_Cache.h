#ifndef Data_Cache_H
#define Data_Cache_H

#include "main.h"


typedef struct
{
    uint8_t start_flag; // 起始标志(0xAA)
    uint8_t ctrl_flags; // 控制位(见下方说明)
    uint8_t packet_id; // 包序列号(0-65535循环)
    uint8_t data_len;  // 数据长度(最大194)
    uint8_t data[200];  // 数据载荷
//    uint16_t crc16;     // CRC16校验
} SerialPacket;

// uint8_t Command_Handle_Buff[2000] = {0};        //数据数据

uint8_t Command_Handle_Data(uint8_t *data, uint8_t length) ;
uint16_t calc_crc16(const void* data, size_t len) ;
uint8_t Command_Send_Data(uint8_t *input, int total_len, SerialPacket *output);
uint8_t CommandBuffer_Write(uint8_t *data, uint8_t length);
uint8_t CommandBuffer_GetCommand(uint8_t *command);
#endif // COMMAND_H