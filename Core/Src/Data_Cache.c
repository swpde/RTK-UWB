/**
 * **************************************************************
 * @file        : Data_Cache.c
 * @author      : Data_Cache
 * @brief       : 利用循环缓冲区实现指令数据缓存
 * **************************************************************
 * @details
 * 指令格式: [包头 ] [数据长度 1字节] [数据 n字节] [校验和 1字节]
 *
 */
#include "Data_Cache.h"
#include "stdio.h"
#include "string.h"
#include <math.h>
// 指令的最小长度
#define COMMAND_MIN_LENGTH 4

// 循环缓冲区大小
#define BUFFER_SIZE 128
// 循环缓冲区
static uint8_t buffer[BUFFER_SIZE];
// 循环缓冲区读索引
static uint8_t readIndex = 0;
// 循环缓冲区写索引
static uint8_t writeIndex = 0;

#define FLAG_SYN (1 << 0)        // 建立连接
#define FLAG_FRAG_START (1 << 1) // 分片开始
#define FLAG_FRAG_END (1 << 2)   // 分片结束
#define FLAG_FIN (1 << 3)        // 结束连接



/**
 * @brief 增加读索引
 * @param length 要增加的长度
 */
void CommandBuffer_AddReadIndex(uint8_t length) {
  readIndex += length;
  readIndex %= BUFFER_SIZE;
}

/**
 * @brief 读取第i位数据 超过缓存区长度自动循环
 * @param i 要读取的数据索引
 */

uint8_t CommandBuffer_Read(uint8_t i) {
  uint8_t index = i % BUFFER_SIZE;
  return buffer[index];
}

/**
 * @brief 计算未处理的数据长度
 * @return 未处理的数据长度
 * @retval 0 缓冲区为空
 * @retval 1~BUFFER_SIZE-1 未处理的数据长度
 * @retval BUFFER_SIZE 缓冲区已满
 */
uint8_t CommandBuffer_GetLength() {
  // 读索引等于写索引时，缓冲区为空
  if (readIndex == writeIndex) {
    return 0;
  }
  // 如果缓冲区已满,返回BUFFER_SIZE
  if (writeIndex + 1 == readIndex || (writeIndex == BUFFER_SIZE - 1 && readIndex == 0)) {
    return BUFFER_SIZE;
  }
  // 如果缓冲区未满,返回未处理的数据长度
  if (readIndex < writeIndex) {
    return writeIndex - readIndex;
  } else {
    return BUFFER_SIZE - readIndex + writeIndex;
  }
}

/**
 * @brief 计算缓冲区剩余空间
 * @return 剩余空间
 * @retval 0 缓冲区已满
 * @retval 1~BUFFER_SIZE-1 剩余空间
 * @retval BUFFER_SIZE 缓冲区为空
 */
uint8_t CommandBuffer_GetRemain() { return BUFFER_SIZE - CommandBuffer_GetLength(); }

/**
 * @brief 向缓冲区写入数据
 * @param data 要写入的数据指针
 * @param length 要写入的数据长度
 * @return 写入的数据长度
 */
uint8_t CommandBuffer_Write(uint8_t *data, uint8_t length) {
  // 如果缓冲区不足 则不写入数据 返回0
  if (CommandBuffer_GetRemain() < length) {
    return 0;
  }
  // 使用memcpy函数将数据写入缓冲区
  if (writeIndex + length <= BUFFER_SIZE) {
    memcpy(buffer + writeIndex, data, length);
    writeIndex += length;
  } else {
    uint8_t firstLength = BUFFER_SIZE - writeIndex;
    memcpy(buffer + writeIndex, data, firstLength);
    memcpy(buffer, data + firstLength, length - firstLength);
    writeIndex = length - firstLength;
  }
  return length;
}

/**
 * @brief 尝试获取一条指令
 * @param command 指令存放指针
 * @return 获取的指令长度
 * @retval 0 没有获取到指令
 */
uint8_t CommandBuffer_GetCommand(uint8_t *command) {
  // 寻找完整指令
  while (1) {
    // 如果缓冲区长度小于COMMAND_MIN_LENGTH 则不可能有完整的指令
    if (CommandBuffer_GetLength() < COMMAND_MIN_LENGTH) {
      return 0;
    }
    // 如果不是包头 则跳过 重新开始寻找
    if (buffer[readIndex] != 0xAA) {
      CommandBuffer_AddReadIndex(1);
      continue;
    }
    // 如果缓冲区长度小于指令长度 则不可能有完整的指令
    uint8_t length = CommandBuffer_Read(readIndex + 1);
    if (CommandBuffer_GetLength() < length) {
      return 0;
    }
    // 如果校验和不正确 则跳过 重新开始寻找
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
      sum += CommandBuffer_Read(readIndex + i);
    }
    if (sum != CommandBuffer_Read(readIndex + length - 1)) {
      CommandBuffer_AddReadIndex(1);
      continue;
    }
    // 如果找到完整指令 则将指令写入command 返回指令长度
    for (uint8_t i = 0; i < length; i++) {
      command[i] = CommandBuffer_Read(readIndex + i);
    }
    CommandBuffer_AddReadIndex(length);
    return length;
  }
}

// 大数据分片封包函数
// 起始标志(0xAA)// 控制位// 包序列号 // 数据长度 // 数据载荷    // CRC16校验
uint8_t Command_Send_Data(uint8_t *input, int total_len, SerialPacket *output)
{
    uint8_t packet_count = 0;
    uint8_t pid = 0;

    for (int i = 0; i < total_len; i += 200)
    {


        SerialPacket *pkt = &output[packet_count++];

        pkt->ctrl_flags = 0;
        pkt->start_flag = 0xAA;
        pkt->packet_id = pid++;
        pkt->data_len = fmin(200, total_len - i);

        // 设置分片标志
        if (i == 0)
            pkt->ctrl_flags |= FLAG_FRAG_START;
        if (i + 200 >= total_len)
            pkt->ctrl_flags |= FLAG_FRAG_END;

        memcpy(pkt->data, input + i, pkt->data_len);
//        pkt->crc16 = calc_crc16(pkt, sizeof(SerialPacket) - 2);
    }
    return packet_count;
}

uint16_t calc_crc16(const void* data, size_t len) {
    uint16_t crc = 0xFFFF;
    const uint8_t* ptr = (const uint8_t*)data;
    while(len--) {
        crc ^= *ptr++ << 8;
        for(int i=0; i<8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}