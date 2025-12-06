//
// Created by hh on 2025/5/7.
//

#ifndef TESTV3_USER_PROTOCOL_H
#define TESTV3_USER_PROTOCOL_H

#include "user_protocol.h"
#include "main.h"


void generate_sensor_json(
        const uint8_t* rtk_data,    // 30字节：经度(15B) + 纬度(15B)
        const uint8_t* uwb_data,    // 45字节：X(15B) + Y(15B) + Z(15B)
        uint8_t id,
        uint8_t indoor_flag,           // 室内标志位
        char* output,               // 输出缓冲区
        size_t buffer_size          // 缓冲区大小
);

void Receive_handle(uint8_t *send_uart2);
 void Send_QueryFrame(UART_HandleTypeDef *huart, uint8_t slave_id);

void Ack_QueryFrame( uint8_t slave_id, uint8_t func,uint8_t *send_uart2);

void Prot_Master_Send(void);
void Prot_Master_receive(void);



// 数据出队（返回1成功，0失败）
uint8_t Dequeue_Data(uint8_t *data, uint16_t len);

//void Prot_Slave_Reserve(UART_HandleTypeDef *huart, uint16_t Size);
void Prot_Slave_Reserve(void);

#endif //TESTV3_USER_PROTOCOL_H
