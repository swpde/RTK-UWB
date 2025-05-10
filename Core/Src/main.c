/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "stdio.h"
#include "user_protocol.h"
#include <ringbuff.h>
#include <stdbool.h>
// #include "Data_Cache.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t aRxBuffer1, aRxBuffer2, aRxBuffer3, aRxBuffer4; // 接收数据存储变量
uint8_t Uart1_RxBuff[100];                              // 数据数据
uint8_t Uart1_Rx_Cnt = 0;                               // 长度
uint8_t G_Uart2_RxBuff[1000] = {0};                     // 数据数据
uint8_t Uart2_Rx_Cnt = 0;                               // 长度
uint8_t G_Uart3_RxBuff[1000] = {0};                     // 数据数据
uint8_t Uart3_Rx_Cnt = 0;                               // 长度
uint8_t Uart4_RxBuff[100];                              // 数据数据
uint8_t Uart4_Rx_Cnt = 0;                               // 长度
// uint8_t Command_Handle_Buff[2000] = {0};        //数据数据

static char main_buf[2000];

volatile uint8_t uart_tx_busy = 0;
volatile uint16_t G_Num_4g = 0;
volatile uint16_t G_Num_lc29 = 0;

extern unsigned char uart1_getok, uart2_getok, uart3_getok;

// extern uint8_t Dequeue_Data(uint8_t *data, uint16_t len);
extern ringbuff_t p_tx_rb;

struct
{
    uint8_t Receive_End;
    uint8_t Receive_Start;
    uint8_t Receive_Count;
    uint16_t Receive_last_length;
} Receive_Handle;
extern unsigned int DMArx_len;
// GPS模块的经纬度数据值
struct
{
    char Latitude[20];  // 经度原数据
    char longitude[20]; // 纬度源数据
    char Latitudess[3]; // 整数部分
    char longitudess[2];
    char Latitudedd[10]; // 小数点部分
    char longitudedd[10];
    float Latitudeddff;
    float longitudeddff;
    char Latitudeddtr[12]; // 小数点部分
    char longitudeddtr[12];
    char TrueLatitude[50];  // 转换过数据
    char Truelongitude[50]; // 转换过数据
    char buffer[200];
    char RTKflag;
    char GGAdata[100];
    // char ALLNEMAdata[2000];
} LongLatidata;
// #pragma  pack (1)
struct ringbuff my_ringbuff = {
    main_buf,
    sizeof(main_buf),
    0,
    0,
    0,
    0, 0, 0

};
// #pragma pack ()

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
//    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
//    return ch;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    MX_DMA_Init();
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_USART4_UART_Init();
    MX_TIM3_Init();
    MX_TIM6_Init();
    /* USER CODE BEGIN 2 */
    OLED_Init();
    OLED_Clear();
    HAL_Delay(500);
    OLED_SHOWAHT20();

    // HAL_UART_Receive_DMA(&huart2, G_Uart2_RxBuff, 200);

    int num = 0;
    int num2 = 0;

    char buf[3];
    char buf2[12] = {0};
    bool send_mirror = true;

    uint8_t send_data[] = {0};
    uint8_t output_data[] = {0};

    //    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, 2000);
    //    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, 2000);
    //
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, Uart4_RxBuff, sizeof(Uart4_RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, sizeof(G_Uart3_RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, sizeof(G_Uart2_RxBuff));

//    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_Base_Start_IT(&htim6);

    static unsigned char ledState = 0;
    // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
    // __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

    // HAL_UART_Receive_DMA();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        if (ledState == 0)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        else
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        ledState = !ledState;

        // 预计算所需封包数
        //        const int num_packets = (sizeof (send_data) + 199) / 200;
        //        SerialPacket packets[num_packets];
        //        Command_Send_Data(send_data,sizeof (send_data),packets);
        //        HAL_UART_Transmit_DMA(&huart2, packets, sizeof (packets));

        //        ring_buff_v1(command);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        //        num++;
        //        sprintf(buf, "%d", num);
        //        OLED_ShowString(90, 7, (u8 *) buf, sizeof(buf));
        HAL_Delay(500);
        //        HAL_Delay(250);

        //        ringbuff_getdata(&my_ringbuff, main_buf, ringbuff_data_len(&my_ringbuff));
        //
        //        HAL_UART_Transmit_DMA(&huart2, main_buf, ringbuff_data_len(&my_ringbuff));
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 16;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

// 串口空闲中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    char buf[4];
    // 串口2  LORA
    if (huart->Instance == USART2)
    {
        USART2_Callback(huart, Size);
    }
    // 串口3 RTK
    if (huart->Instance == USART3)
        if (Size != 0)
        {
            {

                ringbuff_putdata(&my_ringbuff, G_Uart3_RxBuff, Size);
                Receive_Handle.Receive_End = 1;
                G_Num_lc29++;
                sprintf(buf, "%d", G_Num_lc29);
                OLED_ShowString(55, 7, (u8 *)buf, sizeof(buf));

                HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, sizeof(G_Uart3_RxBuff));
                __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
            }
        }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    //    char buf[12];
    uint8_t main_buff[2000];
    if (htim == &htim6)
    {
        TIM6_PeriodElapsed_Callback();
    }
    // 1s 定时发送 rtk ->  Loar
    if (htim == &htim3)
    {

        uint16_t lenght = ringbuff_getdata_all(&my_ringbuff, main_buff);

//        if (uart_tx_busy)
//        {
//            Enqueue_Data(main_buff, lenght); // 入队
//        }
//        else
        {

            HAL_UART_Transmit_DMA(&huart2, main_buff, lenght);
            uart_tx_busy = 1;
        }
    }

    // 500ms 定时发送 轮询 从机设备

}

void OLED_SHOWAHT20(void)
{

    OLED_ShowString(18, 0, "mzhLon:", 8);
    OLED_ShowString(18, 2, "4g", 8);
    OLED_ShowString(18, 4, "mzhLat:", 8);
    OLED_ShowString(18, 7, "lc29", 8);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{

    if (huart == &huart3)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, sizeof(G_Uart3_RxBuff));
        OLED_ShowString(100, 2, "er3", 6);
    }
    if (huart == &huart1)
    {
        OLED_ShowString(100, 2, "er1", 6);

    }
    if (huart == &huart2)
    {
        // 解锁 UART 句柄
        //        __HAL_UNLOCK(huart);
        OLED_ShowString(100, 2, "er2", 6);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, sizeof(G_Uart2_RxBuff));

    }
}
// DMA发送完成中断回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uart_tx_busy = 0; // 释放忙标志

        // 检查队列是否有待发送数据
        uint8_t data[2000]; // 临时缓冲区
        uint16_t len = ringbuff_data_len(p_tx_rb);

        if (len > 0)
        {
            if (Dequeue_Data(data, len))
            {
                HAL_UART_Transmit_DMA(&huart2, data, len);
                uart_tx_busy = 1;
            }
        }
    }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
