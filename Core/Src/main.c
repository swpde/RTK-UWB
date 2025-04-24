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
#include <string.h>
#include <ringbuff.h>
#include <stdbool.h>
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "stdio.h"
//#include "Data_Cache.h"

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
uint8_t aRxBuffer1, aRxBuffer2, aRxBuffer3, aRxBuffer4;            //接收数据存储变量
uint8_t Uart1_RxBuff[100];        //数据数据
uint8_t Uart1_Rx_Cnt = 0;        //长度
uint8_t Uart2_RxBuff[1000];        //数据数据
uint8_t Uart2_Rx_Cnt = 0;        //长度
uint8_t Uart3_RxBuff[1000];        //数据数据
uint8_t Uart3_Rx_Cnt = 0;        //长度
uint8_t Uart4_RxBuff[100];        //数据数据
uint8_t Uart4_Rx_Cnt = 0;        //长度
//uint8_t Command_Handle_Buff[2000] = {0};        //数据数据


static char main_buf[2000];


int num_lora = 0;
int num_lc29 = 0;


extern unsigned char uart1_getok, uart2_getok, uart3_getok;

struct {
    uint8_t Receive_End;
    uint8_t Receive_Start;
    uint8_t Receive_Count;
    uint16_t Receive_last_length;
} Receive_Handle;
extern unsigned int DMArx_len;
//GPS模块的经纬度数据值
struct {
    char Latitude[20];//经度原数据
    char longitude[20];//纬度源数据
    char Latitudess[3];//整数部分
    char longitudess[2];
    char Latitudedd[10];//小数点部分
    char longitudedd[10];
    float Latitudeddff;
    float longitudeddff;
    char Latitudeddtr[12];//小数点部分
    char longitudeddtr[12];
    char TrueLatitude[50];//转换过数据
    char Truelongitude[50];//转换过数据
    char buffer[200];
    char RTKflag;
    char GGAdata[100];
    char Statusdata[100];
//char ALLNEMAdata[2000];
} LongLatidata;
//#pragma  pack (1)
struct ringbuff my_ringbuff = {
        main_buf,
        sizeof(main_buf),
        0,
        0,
        0,
        0, 0, 0

};
//#pragma pack ()

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
PUTCHAR_PROTOTYPE {
/* Place your implementation of fputc here */
/* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart2, (uint8_t *) &ch, 1, 0xFFFF);
    return ch;
}



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void ring_buff_v1(uint8_t *command);

void ring_buff_v2(struct ringbuff *my_ringbuff, const char *msg, int i);

void master(bool send_mirror);

void show_rtk();

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    MX_DMA_Init();

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_USART4_UART_Init();
    /* USER CODE BEGIN 2 */
    OLED_Init();
    OLED_Clear();
    HAL_Delay(500);
    OLED_SHOWAHT20();

    int num = 0;
    char buf[3];
    uint8_t send_mirror = 0;

    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, Uart4_RxBuff, sizeof(Uart4_RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, sizeof(Uart3_RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));

    //HAL_UART_Receive_DMA();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        if (Receive_Handle.Receive_End == 1) {

            if (send_mirror == 2) {

                uint16_t lenght = ringbuff_getdata_all(&my_ringbuff, main_buf);
                HAL_Delay(250);
                HAL_UART_Transmit_DMA(&huart2, main_buf, lenght);
                Receive_Handle.Receive_End = 0;
                Receive_Handle.Receive_Count = 0;
                Receive_Handle.Receive_last_length = 0;

                //显示rtk是否进入收敛
                show_rtk();
            }
            send_mirror++;
            if (send_mirror == 3) send_mirror = 0;



        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
//        num++;
//        sprintf(buf, "%d", num);
//        OLED_ShowString(90, 7, (u8 *) buf, sizeof(buf));
        HAL_Delay(50);


    }
    /* USER CODE END 3 */
}

void show_rtk() {
    char *strx;
    char *p;
    uint16_t i;
    char num[2];
    strx = strstr((const char *) Uart3_RxBuff, (const char *) "$PQTMSVINSTATUS");//返回$GNGGA
    if (strx) {
        for ( i = 0;; i++) {
            if (strx[i + 1] == '*')
                break;
            LongLatidata.Statusdata[i] = strx[i];
        }
        LongLatidata.Statusdata[i] = 0;
        p = strtok(strx, ",");
        p = strtok(NULL, ",");
        p = strtok(NULL, ",");
        p = strtok(NULL, ",");
        memcpy( &num , p, 1);

        if (num[0] == 92)
            OLED_ShowString(25, 6, "FLOAT RTK", 12);
    }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the peripherals clocks
    */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

// 串口接收完成（收到2个字节）中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//    if (huart->Instance == USART2)
//    {
//
//        // 将收到的数据返回（中断发送）
////        HAL_UART_Transmit_IT(&huart2, Uart2_RxBuff, 2);
//        // 重新开启中断接收
//        HAL_UART_Receive_IT(&huart2, Uart2_RxBuff, 2);
//    }
}

//串口空闲中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    char buf[3];

    //串口2接收回调函数
    if (huart->Instance == USART2) {

        HAL_UART_Transmit(&huart1, Uart2_RxBuff, Size, 500);
        num_lora++;
        sprintf(buf, "%d", num_lora);
        OLED_ShowString(55, 2, (u8 *) buf, sizeof(buf));

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

    }
    //串口3  接收到差分数据 发送 给串口2 lorA
    if (huart->Instance == USART3)
        if (Size != 0) {
            {
                ringbuff_putdata(&my_ringbuff, Uart3_RxBuff, Size);
                //串口1接收，打印调试
//                HAL_UART_Transmit(&huart1, Uart3_RxBuff, Size, 100);
                Receive_Handle.Receive_End = 1;
                num_lc29++;
                sprintf(buf, "%d", num_lc29);
                OLED_ShowString(55, 7, (u8 *) buf, sizeof(buf));

                HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, sizeof(Uart3_RxBuff));
                __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

            }
        }
//    if (huart == &huart4) {
//        //uint8_t length = CommandBuffer_Write(Uart2_RxBuff, Size);
//
//        HAL_UART_Transmit_DMA(&huart2, Uart4_RxBuff, Size);
////        OLED_ShowString(18, 5, "88", 8);
//        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, Uart4_RxBuff, 2000);
////        __HAL_DMA_DISABLE_IT(&hdma_usart4_rx, DMA_IT_HT);
//
//    }

}

//// 不定长数据接收完成回调函数
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//    if (huart->Instance == USART2)
//    {
//        // 使用DMA将接收到的数据发送回去
//        HAL_UART_Transmit_DMA(&huart2, Uart2_RxBuff, Size);
//        // 重新启动接收，使用Ex函数，接收不定长数据
//        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));
//        // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
//        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
//    }
//}


void OLED_SHOWAHT20(void) {

    OLED_ShowString(18, 0, "mzhLon:", 8);
    OLED_ShowString(18, 2, "4g", 8);
    OLED_ShowString(18, 4, "mzhLat:", 8);
    OLED_ShowString(18, 7, "lc29", 8);

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {


    if (huart == &huart3) {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, sizeof(Uart3_RxBuff));
        OLED_ShowString(80, 2, "er3", 8);

    }
    if (huart == &huart1) {
        OLED_ShowString(80, 2, "er1", 8);

// 解锁 UART 句柄
//        __HAL_UNLOCK(huart);
// 重新启动 UART 接收中断
//        HAL_UART_Receive_IT(&huart1, Tx_M_Buff_Temp[0], 1);
    }
    if (huart == &huart2) {
// 解锁 UART 句柄
//        __HAL_UNLOCK(huart);
        OLED_ShowString(80, 2, "er2", 8);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));

// 重新启动 UART 接收中断
//        HAL_UART_Receive_IT(&huart1, Tx_M_Buff_Temp[0], 1);
    }


}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {

    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
