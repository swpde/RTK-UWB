#ifndef Data_Cache_H
#define Data_Cache_H

#include "main.h"


typedef struct
{
    uint8_t start_flag; // ��ʼ��־(0xAA)
    uint8_t ctrl_flags; // ����λ(���·�˵��)
    uint8_t packet_id; // �����к�(0-65535ѭ��)
    uint8_t data_len;  // ���ݳ���(���194)
    uint8_t data[200];  // �����غ�
//    uint16_t crc16;     // CRC16У��
} SerialPacket;

// uint8_t Command_Handle_Buff[2000] = {0};        //��������

uint8_t Command_Handle_Data(uint8_t *data, uint8_t length) ;
uint16_t calc_crc16(const void* data, size_t len) ;
uint8_t Command_Send_Data(uint8_t *input, int total_len, SerialPacket *output);
uint8_t CommandBuffer_Write(uint8_t *data, uint8_t length);
uint8_t CommandBuffer_GetCommand(uint8_t *command);
#endif // COMMAND_H