//
// Created by hh on 2025/5/7.
//

#ifndef TESTV3_USER_PROTOCOL_H
#define TESTV3_USER_PROTOCOL_H


void Send_QueryFrame(UART_HandleTypeDef *huart, uint8_t slave_id);
void Prot_Master_Send(void);


uint8_t Enqueue_Data(uint8_t *data, uint16_t len);


// 数据出队（返回1成功，0失败）
uint8_t Dequeue_Data(uint8_t *data, uint16_t len);

void Prot_Slave_Reserve(UART_HandleTypeDef *huart, uint16_t Size);


#endif //TESTV3_USER_PROTOCOL_H
