
/**
 * **************************************************************
 * @file        : ringbuff.c
 * @author      : swp
 * @date        :4/14
 * @brief       : 利用循环缓冲区实现指令数据缓存
 * **************************************************************
 * @details
 *
 *
 */
#include "ringbuff.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "string.h"

#include <math.h>
#include <usart.h>
#include <malloc.h>

static uint8_t buffer[2000];


//返回剩余缓存大小
uint16_t ringbuff_data_space_len(ringbuff_t p_rb) {
    if (p_rb->readPos == p_rb->write_pos) {
        if (p_rb->read_mirror == p_rb->write_mirror)
            return p_rb->buff_size;
        else
            return 0;
    } else {
        if (p_rb->read_mirror == p_rb->write_mirror) {
            return p_rb->buff_size - (p_rb->write_pos - p_rb->readPos);
        } else {
            return (p_rb->readPos - p_rb->write_pos);
        }


    }
//        return RINGBUFF_HALFFULL;
//
//
//
//
//    return  (p_rb)->buff_size - ringbuff_data_len(p_rb);
//

}


//返回缓存大小
int16_t ringbuff_data_len(ringbuff_t p_rb) {
    switch (ringbuff_state_get(p_rb)) {
        case RINGBUFF_EMPTY:
            return p_rb->exist = 0;
        case RINGBUFF_FULL:
            return p_rb->exist = p_rb->buff_size;
        case RINGBUFF_HALFFULL:
            if (p_rb->read_mirror == p_rb->write_mirror) {
                return p_rb->exist = p_rb->write_pos - p_rb->readPos;
            } else {
                return p_rb->exist = p_rb->buff_size - (p_rb->readPos - p_rb->write_pos);
            }
    }
    // never reach here
    return p_rb->buff_size;
}

int32_t ringbuff_getchar(ringbuff_t p_rb, uint8_t *ch) {
    assert(p_rb != NULL);
    // empty buffer
    if (!ringbuff_data_len(p_rb)) return 0;

    *ch = ((char *) p_rb->buf)[p_rb->readPos];

    if (p_rb->readPos == p_rb->buff_size - 1) {
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->readPos = 0;
    } else {
        p_rb->readPos++;
    }

    return 1;
}

int32_t ringbuff_putchar(ringbuff_t p_rb, const uint8_t ch) {
    assert(p_rb != NULL);
    //判断剩余长度
    if (!ringbuff_data_space_len(p_rb)) {
        // full buffer
        return 0;
    }
    //如果有剩余就写入一位
    ((char *) p_rb->buf)[p_rb->write_pos] = ch;
    //写满重新放到原点0
    if (p_rb->write_pos == p_rb->buff_size - 1) {
        p_rb->write_mirror = ~p_rb->write_mirror;
        p_rb->write_pos = 0;
    } else p_rb->write_pos++;

    return 1;
}

/**
 * @brief 向缓冲区取 出数据
 * @param p_rb 要传入的数据指针
 * @param data 要写入的数据
 * @param length 要写入的数据长度
 * @return bool
 */
int8_t ringbuff_getdata(ringbuff_t p_rb, uint8_t *data, uint16_t length) {
    assert(p_rb != NULL);
    // empty buffer
    if (!ringbuff_data_len(p_rb)) return 0;

    // 使用memcpy函数将缓冲区读出
    if (p_rb->readPos + length < p_rb->buff_size) {
        memcpy(data, buffer + (p_rb->readPos), length);
        p_rb->readPos += length;
    } else if (p_rb->readPos + length == p_rb->buff_size) {
        memcpy(data, buffer + p_rb->readPos, length);
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->readPos = 0;
    } else {
        uint8_t firstLength = p_rb->buff_size - p_rb->readPos;
        memcpy(data, buffer + p_rb->readPos, firstLength);
        memcpy(data + firstLength, buffer, length);
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->readPos = length - firstLength;
    }
    return 1;
}

/**
 * @brief 向缓冲区写入数据
 * @param p_rb 要传入的数据指针
 * @param data 要写入的数据
 * @param length 要写入的数据长度
 * @return bool
 */
int8_t ringbuff_putdata(ringbuff_t p_rb, uint8_t *data, uint16_t length) {
    assert(p_rb != NULL);
    // 如果缓冲区不足 则不写入数据 返回0
    if (length == 0) {
        printf("fail \n");
        return 0;
    }
    if ((ringbuff_data_space_len(p_rb)) < length) {
        printf("fail \n");
        return 0;
    }

    // 使用memcpy函数将数据写入缓冲区
    if (p_rb->write_pos + length < p_rb->buff_size) {
        memcpy(buffer + (p_rb->write_pos), data, length);
        p_rb->write_pos += length;
    } else if (p_rb->write_pos + length == p_rb->buff_size) {
        memcpy(buffer + p_rb->write_pos, data, length);
        p_rb->write_mirror = ~p_rb->write_mirror;
        p_rb->write_pos = 0;     //写满重新放到原点0
    } else {
        uint16_t firstLength = p_rb->buff_size - p_rb->write_pos;
        p_rb->write_mirror = ~p_rb->write_mirror;

        memcpy(buffer + p_rb->write_pos, data, firstLength);
        memcpy(buffer, data + firstLength, (length - firstLength));
        p_rb->write_pos = length - firstLength;
    }
    return 1;
}


/**
 * @brief 向缓冲区写入数据
 * @param p_rb 要传入的数据指针
 * @param data 要写入的数据
 * @param length 要写入的数据长度
 * @return bool
 */
uint16_t ringbuff_getdata_all(ringbuff_t p_rb, uint8_t *data) {
    assert(p_rb != NULL);
    // 如果缓冲区不足 则不写入数据 返回0

    uint16_t length = p_rb->buff_size - ringbuff_data_space_len(p_rb);
//    printf("读取: %d \n", length);

    p_rb->exist = length;
    if (length == 0) {
        printf("fail");
        return 0;
    }
    // 使用memcpy函数将缓冲区读出
    if (p_rb->readPos + length < p_rb->buff_size) {
        memcpy(data, buffer + (p_rb->readPos), length);
        p_rb->readPos += length;
    } else if (p_rb->readPos + length == p_rb->buff_size) {
        memcpy(data, buffer + p_rb->readPos, length);
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->readPos = 0;
    } else {
        uint16_t firstLength = p_rb->buff_size - p_rb->readPos;
        memcpy(data, buffer + p_rb->readPos, firstLength);
        memcpy(data + firstLength, buffer, length);
        p_rb->read_mirror = ~p_rb->read_mirror;
        p_rb->readPos = length - firstLength;
    }
    return length;
}


int32_t ringbuff_peekchar(ringbuff_t p_rb, uint8_t *ch) {
    assert(p_rb != NULL);

    if (!ringbuff_data_len(p_rb)) {
        // empty buffer
        return 0;
    }

    *ch = ((char *) p_rb->buf)[p_rb->readPos];

    return 1;
}

// 起始标志(0xAA)// 功能码// 总包数 //包序列 // 数据长度 // 数据载荷    // CRC16校验（没用）
/**
 * @brief 由于lora限制单包数据不能超过260字节，否则会丢包，
 *         所以将发送数据拆包
 * @param send_data 要传入的数据指针
 * @param total_len 要写入的数据总长度
 * @param chunk_len 每组长度
 * @param outputoutput 输出数据指针
 * @return
 */
uint8_t Command_Send_Data(uint8_t *send_data, uint16_t total_len, uint16_t chunk_len) {
    uint8_t packet_count = 0;
    uint8_t pid = 0;
    uint16_t remaining = 0;
    uint16_t chunk_size = chunk_len - 5;   // 每组大小
    uint16_t num_chunks = (total_len + chunk_size - 1) / chunk_size;

    for (uint8_t i = 0; i < (uint8_t) num_chunks; i++) {
        uint16_t start = i * chunk_size;
        remaining = total_len - start;
        uint16_t copy_size = (remaining < chunk_size) ? remaining : chunk_size;


        uint8_t pkt[chunk_len];
        pkt[0] = 0x6B;
        if (i == 0) pkt[1] = 0x02;
        else if (i + 1 == num_chunks) pkt[1] = 0x04;
        else pkt[1] = 0;
        pkt[2] = num_chunks;
        pkt[3] = pid++;
        pkt[4] = copy_size;
        // 安全复制数据
        memcpy(pkt + 5, send_data + start, copy_size);
        HAL_UART_Transmit(&huart2, pkt, copy_size + 5, 100);
        HAL_Delay(100);
    }

    return packet_count;
}


/**
 * @brief 由于lora限制单包数据不能超过260字节，否则会丢包，
 *         所以将发送数据拆包
 * @param send_data 要传入的数据指针
 * @param total_len 要写入的数据总长度
 * @param chunk_len 每组长度
 * @param outputoutput 输出数据指针
 * @return
 */
uint8_t Command_slave_Send_Data(uint8_t *send_data, uint16_t total_len, uint16_t chunk_len) {
    uint8_t packet_count = 0;
    uint8_t pid = 0;
    uint16_t remaining = 0;
    uint16_t chunk_size = chunk_len - 5;   // 每组大小
    uint16_t num_chunks = (total_len + chunk_size - 1) / chunk_size;

    for (uint8_t i = 0; i < (uint8_t) num_chunks; i++) {
        uint16_t start = i * chunk_size;
        remaining = total_len - start;
        uint16_t copy_size = (remaining < chunk_size) ? remaining : chunk_size;


        uint8_t pkt[chunk_len];
        pkt[0] = 0x6B;
        if (i == 0) pkt[1] = 0x02;
        else if (i + 1 == num_chunks) pkt[1] = 0x04;
        else pkt[1] = 0;
        pkt[2] = num_chunks;
        pkt[3] = pid++;
        pkt[4] = copy_size;
        // 安全复制数据
        memcpy(pkt + 5, send_data + start, copy_size);
        HAL_UART_Transmit(&huart2, pkt, copy_size + 5, 100);
        HAL_Delay(100);
    }

    return packet_count;
}


//发送所有数据
uint8_t Command_Send_Data_t(uint8_t *send_data, uint8_t packets_num, uint16_t total_len) {
    uint8_t packet_count = 0;
    uint8_t pid = 0;
    uint8_t pkt[200] = {0};

    uint16_t total_size = total_len;   // 总数据长度
    uint16_t chunk_size = 200;   // 每组大小
    uint16_t num_chunks = (total_size + chunk_size - 1) / chunk_size;

    for (uint8_t i = 0; i < num_chunks; i++) {
        uint16_t start = i * chunk_size;
        uint16_t remaining = total_size - start;
        uint16_t copy_size = (remaining < chunk_size) ? remaining : chunk_size;

        // 安全复制数据
        memcpy(pkt, send_data + start, copy_size);
        HAL_UART_Transmit(&huart2, pkt, copy_size, 100);
        HAL_Delay(100);
    }
    return packet_count;
}

uint8_t Command_Analysis_Data(uint8_t *receive_data, uint16_t total_len, uint16_t Receive_last_length) {
    uint8_t copy_size = 0;
    uint16_t chunk_size = 195;   // 每组要取大小
    uint16_t num_chunks = (total_len + chunk_size - 1) / chunk_size;

    uint16_t send_size= (num_chunks-1)*195+Receive_last_length;
    uint8_t send_data[send_size] ;

    for (uint8_t i = 0; i < (uint8_t) num_chunks; i++) {

        if (i + 1 == num_chunks) copy_size = Receive_last_length;
        else copy_size = chunk_size;
        memcpy(send_data + i * 195, receive_data + i * 200 + 5, copy_size);

    }

    HAL_UART_Transmit(&huart1, send_data, send_size, 100);
    HAL_Delay(50);
    HAL_UART_Transmit(&huart3, send_data, send_size, 100);
    return 1;
}


void ringbuff_debug(ringbuff_t p_rb) {
//    printf("*************ringbuff debug***********************\n");
//    printf("ringbuff buf:\n");
//    for (int i = 0; i < p_rb->buff_size; i++) {
//        printf("%c", ((char *) p_rb->buf)[i]);
//    }
//    printf("\n");
//    printf("ringbuff buf end---\n");
    printf("read_mirror  = %d\n", p_rb->read_mirror);
    printf("readPos      = %d\n", p_rb->readPos);
    printf("write_mirror = %d\n", p_rb->write_mirror);
    printf("write_pos    = %d\n", p_rb->write_pos);
//    printf("exist = %d\n", p_rb->exist);

//    printf("*************ringbuff debug*********************** end---\n");
}