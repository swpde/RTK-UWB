//
// Created by hh on 2025/5/7.
//
#include <usart.h>
#include <string.h>
#include "main.h"
#include "user_protocol.h"
#include "ringbuff.h"

//------------------- 配置参数 -------------------//
// 从机参数
#define SLAVE_ID 0 // 本设备ID

// 主机参数
#define MAX_SLAVES 7      // 最大从站数量
#define POLL_INTERVAL 500 // 轮询间隔(ms)
#define MAX_RETRIES 3     // 最大重试次数

// 协议参数
#define PROT_HEADER 0xAA     // 帧头（2字节）
#define PROT_FUNC_QUERY 0x03 // 查询功能码
#define PROT_FUNC_ACK 0x07   // 应答功能码
#define PROT_SLAVE_COUNT 5   // 从站数量
#define POLL_INTERVAL_MS 500 // 轮询间隔

// 消息队列
#define TX_QUEUE_SIZE 2000 // 调整队列容量以匹配 ringbuff 的缓冲区大小

struct ringbuff tx_ring_buffer;
ringbuff_t p_tx_rb = &tx_ring_buffer;

typedef struct {
    uint8_t header;           // 帧头 0xAA55
    uint8_t function;         // 功能码
    uint8_t slave_id;         // 从站地址
    char x_coord[15];         // X坐标 15B
    char y_coord[15];         // Y坐标 15B
    char z_coord[15];         // Z坐标 15B
    uint16_t signal_strength; // 信号强度 2B
} UART_Frame;

// 从站设备状态
typedef struct {
    uint8_t id;             // 从站ID
    uint8_t retries;        // 当前重试次数
    uint8_t last_comm_time; // 最后通信时间戳
    uint8_t data[100];      // 最新接收数据
} SlaveDevice;

//------------------- 全局变量 -------------------//

SlaveDevice slave_list[MAX_SLAVES] = {
        {1, 0, 0, {0}},
        {2, 0, 0, {0}},
        {3, 0, 0, {0}},
        {4, 0, 0, {0}},
        {5, 0, 0, {0}},
        {6, 0, 0, {0}},
        {7, 0, 0, {0}},
};

volatile uint8_t current_slave = 0; // 当前轮询的从站索引
// volatile uint8_t uart_tx_busy = 0; // DMA发送状态标志
volatile uint8_t timer_tick = 0; // 定时器触发标志

//------------------- 构造并发送查询帧 -------------------//
void Send_QueryFrame(UART_HandleTypeDef *huart, uint8_t slave_id) {
    UART_Frame frame = {
            .header = PROT_HEADER,
            .function = PROT_FUNC_QUERY,
            .slave_id = slave_id,
    };

    //    if (uart_tx_busy)
    //    {
    //        Enqueue_Data((uint8_t *)&frame, sizeof(frame)); // 入队
    //    }
    //    else
    {
        HAL_UART_Transmit_DMA(&huart2, (uint8_t *) &frame, sizeof(frame));
        uart_tx_busy = 1;
    }

    // 更新从站状态
//    slave_list[current_slave].last_comm_time = HAL_GetTick();
}

void Ack_QueryFrame(UART_HandleTypeDef *huart, uint8_t slave_id, uint8_t func) {
    if (!uart_tx_busy) {
        uart_tx_busy = 1;


        UART_Frame frame = {
                .header = PROT_HEADER,
                .function = func,
                .slave_id = slave_id,
                //                .payload = {0x01, 0x02} // 荷载
        };

        // 发送原始字节流
        HAL_UART_Transmit_DMA(huart, (uint8_t *) &frame, sizeof(frame));

        // 更新从站状态
        slave_list[current_slave].last_comm_time = HAL_GetTick();
    }
}

//------------------- 从站列表维护函数 -------------------//
void Maintain_SlaveList(void) {
    static uint32_t last_check = 0;
    uint32_t now = HAL_GetTick();

    // 每1秒检查一次通信状态
    if (now - last_check > 1000) {
        last_check = now;

        for (int i = 0; i < MAX_SLAVES; i++) {
            // 超时未响应则增加重试计数
            if (slave_list[i].last_comm_time > 0 &&
                (now - slave_list[i].last_comm_time) > (POLL_INTERVAL * MAX_RETRIES)) {

                if (slave_list[i].retries < MAX_RETRIES) {
                    slave_list[i].retries++;
                } else {
                    // 标记从站离线（示例：清空数据）
                    memset(slave_list[i].data, 0, sizeof(slave_list[i].data));
                }
            }
        }
    }
}

void Prot_Master_Send(void) {

    Send_QueryFrame(&huart2, slave_list[current_slave].id); // 发送当前从站查询
    current_slave = (current_slave + 1) % MAX_SLAVES;       // 切换到下一个从站
    //    Maintain_SlaveList(); // 维护从站列表状态
}

void Prot_Slave_Send(void) {
}

// 帧头1 + 功能码1 + ID1 + 荷载
void Prot_Slave_Reserve(UART_HandleTypeDef *huart, uint16_t Size) {
    // 验证帧头
    if (G_Uart3_RxBuff[0] != PROT_HEADER)
        return;
    // 检查从站ID
    if (G_Uart3_RxBuff[2] != SLAVE_ID)
        return;

    // 解析功能码
    uint8_t func_code = G_Uart3_RxBuff[1];

    // 根据功能码执行操作
    switch (func_code) {
        case 0x01: // 读数据
            //            Send_Response(func_code, 0x01, 25, 60); // 示例数据
            break;

        case 0x02: // 写数据
        {
            //            if (rx_len >= 5) { // 需要额外2字节参数
            //                uint8_t param1 = rx_buf[3];
            //                uint8_t param2 = rx_buf[4];
            //                Save_Parameters(param1, param2);
            //                Send_Response(func_code, 0x00, 0, 0); // 成功响应
        }
            break;
        case 0x03: // 主站查询
        {
        }
            break;
        default:
            break;
    }
}

// 数据入队（返回1成功，0失败）
uint8_t Enqueue_Data(uint8_t *data, uint16_t len) {
    return ringbuff_putdata(p_tx_rb, data, len);
}

// 数据出队（返回1成功，0失败）
uint8_t Dequeue_Data(uint8_t *data, uint16_t len) {
    return ringbuff_getdata(p_tx_rb, data, len);
}

// 初始化环形缓冲区
void Init_TX_Buffer(void) {
    p_tx_rb->buff_size = TX_QUEUE_SIZE;
    p_tx_rb->read_mirror = 0;
    p_tx_rb->readPos = 0;
    p_tx_rb->write_mirror = 0;
    p_tx_rb->write_pos = 0;
}

//------------------- 接收数据处理 -------------------//
// void USART2_IRQHandler(void) {
//    HAL_UART_IRQHandler(&huart2); // HAL库中断处理
//
//    // 自定义接收逻辑（示例：字节接收）
//    static uint8_t rx_buf[sizeof(UART_Frame)];
//    static uint8_t rx_cnt = 0;
//
//    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
//        rx_buf[rx_cnt++] = huart2.Instance->DR;
//
//        // 检查帧完整性
//        if (rx_cnt >= sizeof(UART_Frame)) {
//            UART_Frame *frame = (UART_Frame *) rx_buf;
//
//            // 验证帧头
//            if (frame->header == 0xAA55) {
//                // 查找对应从站
//                for (int i = 0; i < MAX_SLAVES; i++) {
//                    if (slave_list[i].id == frame->slave_id) {
//                        // 更新数据
//                        memcpy(slave_list[i].data, frame->payload, 2);
//                        slave_list[i].retries = 0; // 重置重试计数
//                        break;
//                    }
//                }
//            }
//
//            rx_cnt = 0; // 重置接收计数器
//        }
//    }
//}