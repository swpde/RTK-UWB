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
#include "../../Application/user_protocol.h"
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
uint8_t G_Uart2_RxBuff[1000] = {0};        //数据数据
uint8_t Uart2_Rx_Cnt = 0;        //长度
uint8_t G_Uart3_RxBuff[2000] = {0};        //数据数据
uint8_t Uart3_Rx_Cnt = 0;        //长度
uint8_t Uart4_RxBuff[100];        //数据数据
uint8_t Uart4_Rx_Cnt = 0;        //长度
//uint8_t Command_Handle_Buff[2000] = {0};        //数据数据


static uint8_t main_buf[2000];


int num_4g = 0;
int num_lc29 = 0;


extern unsigned char uart1_getok, uart2_getok, uart3_getok;
#define WINDOW_SIZE 5  // 时间窗口长度（5次历史记录）

struct {
    uint8_t Receive_End;
    uint8_t Receive_Start;
    uint8_t Receive_Count;
    uint16_t Receive_last_length;
    uint8_t uart3_getok;   //为接收到gps数据
    uint8_t uart4_getok;  //为接收到gps数据
} Receive_Handle;

// DOP滤波器结构体

DOPFilter filter;
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
    char TrueLatitude[15];//转换过数据
    char Truelongitude[15];//转换过数据
    char buffer[200];
    char RTKflag;
    char pdop[5];     //三维位置精度

    float hdop;
    uint8_t GpsStatus;  //gps状态
    uint8_t sat_count;  //卫星数量
    char GGAdata[100];
//char ALLNEMAdata[2000];
} LongLatidata;
//GPS模块的经纬度数据值
struct {
    char uwb_x[15];//x原数据
    char uwb_y[15];//y源数据
    char uwb_z[15];//z源数据
    char uwb_strength[2];//整数部分
    char uwb_flag;
    char uwb_rawdata[100];
} uwbdata;
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

void Receive_handle_signal_strength(uint8_t *send_uart2);

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

    init_filter(&filter);

    //HAL_UART_Receive_DMA(&huart2, G_Uart2_RxBuff, 200);

    int num = 0;
    int num2 = 0;//0022

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

// 使用Ex函数，接收不定长数据
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, sizeof(G_Uart2_RxBuff));




// 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
    // __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

    //HAL_UART_Receive_DMA();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

// 预计算所需封包数
//        const int num_packets = (sizeof (send_data) + 199) / 200;
//        SerialPacket packets[num_packets];
//        Command_Send_Data(send_data,sizeof (send_data),packets);
//        HAL_UART_Transmit_DMA(&huart2, packets, sizeof (packets));

//        ring_buff_v1(command);




        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        sprintf(buf, "%d", num);
        OLED_ShowString(90, 7, (u8 *) buf, sizeof(buf));
        HAL_Delay(500);
//        HAL_UART_Transmit(&huart2,send_Lati_data,100,100);
//        HAL_UART_Transmit(&huart1,send_Lati_data,100,100);

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



//串口空闲中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    char buf[3];
    char buf2[3];
    //串口2接收 lora
    if (huart->Instance == USART2) {

        if (Size != 0) {


            Prot_Slave_Reserve();


            HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, sizeof(G_Uart2_RxBuff));
            __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

        }
    }
    //串口3接收 gps
    if (huart->Instance == USART3) {
        if (Size != 0) {
            {
//                ringbuff_putdata(&my_ringbuff, G_Uart3_RxBuff, Size);
                Receive_Handle.uart3_getok = 1;
                HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, sizeof(G_Uart3_RxBuff));
                __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

            }
        }

    }

    //串口3接收 gps
    if (huart->Instance == USART4) {
        if (Size != 0) {
            {
//                ringbuff_putdata(&my_ringbuff, G_Uart3_RxBuff, Size);
                Receive_Handle.uart4_getok = 1;


                HAL_UARTEx_ReceiveToIdle_DMA(&huart4, Uart4_RxBuff, sizeof(Uart4_RxBuff));
                __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

            }
        }

    }


}

void Receive_handle_signal_strength(uint8_t *send_uart2) {
    if (Receive_Handle.uart4_getok && Receive_Handle.uart3_getok)//gps data is ok
    {
        Receive_Handle.uart4_getok = 0;
        Receive_Handle.uart3_getok = 0;


        update_filter(&filter, , );

        // 获取判断结果
        bool result = is_outdoor(&filter);

        if (atoi(uwbdata.uwb_strength) > 50) {
            Getdata_uwb();
            memcpy(send_uart2, uwbdata.uwb_x, sizeof(uwbdata.uwb_x));
            memcpy(send_uart2 + sizeof(uwbdata.uwb_x), uwbdata.uwb_y, sizeof(uwbdata.uwb_y));
            memcpy(send_uart2 + sizeof(uwbdata.uwb_y), uwbdata.uwb_z, sizeof(uwbdata.uwb_z));
//            HAL_UART_Transmit_DMA(&huart2, send_uart2, 100);
//                    num++;

        } else {
            Getdata_gps();
            memcpy(send_uart2, LongLatidata.TrueLatitude, 50);
            memcpy(send_uart2 + 50, LongLatidata.Truelongitude, 50);
//            HAL_UART_Transmit_DMA(&huart2, send_uart2, 100);
//                    num++;
        }
    } else if (Receive_Handle.uart4_getok == 0 && Receive_Handle.uart3_getok) {
        Receive_Handle.uart3_getok = 0;
        Getdata_gps();
        memcpy(send_uart2, 0, 200);
        memcpy(send_uart2, LongLatidata.TrueLatitude, 15);
        memcpy(send_uart2 + 15, LongLatidata.Truelongitude, 15);
//        HAL_UART_Transmit_DMA(&huart2, send_uart2, 30);
//                num++;
    } else if (Receive_Handle.uart4_getok && Receive_Handle.uart3_getok == 0) {
        Receive_Handle.uart4_getok = 0;
        Getdata_uwb();
        memcpy(send_uart2, uwbdata.uwb_x, sizeof(uwbdata.uwb_x));
        memcpy(send_uart2 + sizeof(uwbdata.uwb_x), uwbdata.uwb_y, sizeof(uwbdata.uwb_y));
        memcpy(send_uart2 + sizeof(uwbdata.uwb_y), uwbdata.uwb_z, sizeof(uwbdata.uwb_z));
//        HAL_UART_Transmit_DMA(&huart2, send_uart2, 100);
//                num++;
    }
}




/*************将原始数据解析出经纬度数据*******************/
//$GNGGA,015032.000,3150.303376,N,11707.855089,E,1,22,1.91,197.8,M,-0.3,M,,*6B
void Getdata_gps(void) {
    unsigned char i;
    char *strx;
    char *p;
    char json[] = "{lon:%d.%06d:lat:%d.%06d}";
    memset(LongLatidata.GGAdata, 0, 100);
    strx = strstr((const char *) G_Uart3_RxBuff, (const char *) "$GNGGA");//返回$GNGGA
    if (strx) {
        for (i = 0;; i++) {
            if (strx[i + 1] == '$')
                break;
            LongLatidata.GGAdata[i] = strx[i];
        }
        LongLatidata.GGAdata[i] = 0;
//        printf(LongLatidata.GGAdata);//mqtt send
        strx = strstr((const char *) LongLatidata.GGAdata, (const char *) "N,");//返回N，表明经纬度数据被正确获取了
        if (strx) {

            memcpy(LongLatidata.buffer, LongLatidata.GGAdata, 100);
            memset(LongLatidata.longitude, 0, 20);
            memset(LongLatidata.Latitude, 0, 20);
            memset(LongLatidata.Truelongitude, 0, 15);
            memset(LongLatidata.TrueLatitude, 0, 15);
            // printf(LongLatidata.buffer);
            p = strtok(LongLatidata.buffer, ",");
            p = strtok(NULL, ",");
            p = strtok(NULL, ",");
            // 	// //  Uart1_SendStr(p);
            memset(LongLatidata.longitude, 0, 11);
            memcpy(LongLatidata.longitude, p, 11);
            //  printf(LongLatidata.longitude);
            p = strtok(NULL, ",");
            p = strtok(NULL, ",");
            memset(LongLatidata.Latitude, 0, 13);
            memcpy(LongLatidata.Latitude, p, 13);
            p = strtok(NULL, ",");
            p = strtok(NULL, ",");
            LongLatidata.GpsStatus = atoi(p);
            p = strtok(NULL, ",");
            LongLatidata.sat_count = atoi(p);
            p = strtok(NULL, ",");
            LongLatidata.hdop = atof(p);


            //printf(LongLatidata.Latitude);
            //	Uart1_SendStr(p);
            strx = strstr((const char *) LongLatidata.GGAdata, (const char *) "E,");//返回E，读取纬度数据11702.5641
            if (strx) {
                LongLatidata.RTKflag = strx[2];
                for (i = 0; i < 3; i++) {
                    LongLatidata.Latitudess[i] = LongLatidata.Latitude[i];
                    LongLatidata.TrueLatitude[i] = LongLatidata.Latitude[i];
                }

                for (i = 3; i < 13; i++)
                    LongLatidata.Latitudedd[i - 3] = LongLatidata.Latitude[i];
                LongLatidata.Latitudedd[i - 3] = 0;
                //LongLatidata.Latitudeddff=atof(LongLatidata.Latitudedd);
                LongLatidata.Latitudeddff = strtod(LongLatidata.Latitudedd, NULL);
                LongLatidata.Latitudeddff /= 60;
                sprintf(LongLatidata.Latitudeddtr, "%0.8f", LongLatidata.Latitudeddff);
                LongLatidata.Latitudeddtr[10] = 0;
                for (i = 1; i < 10; i++) {
                    LongLatidata.TrueLatitude[2 + i] = LongLatidata.Latitudeddtr[i];
                }

                // sscanf(LongLatidata.Latitudedd,"%lf",&LongLatidata.Latitudeddff);//
                //			///////////////////////////////////////////
                for (i = 0; i < 2; i++) {
                    LongLatidata.longitudess[i] = LongLatidata.longitude[i];
                    LongLatidata.Truelongitude[i] = LongLatidata.longitude[i];
                }
                for (i = 2; i < 11; i++)
                    LongLatidata.longitudedd[i - 2] = LongLatidata.longitude[i];
                LongLatidata.longitudedd[i - 2] = 0;
                LongLatidata.longitudeddff = strtod(LongLatidata.longitudedd, NULL);
                LongLatidata.longitudeddff /= 60;
                sprintf(LongLatidata.longitudeddtr, "%0.8f", LongLatidata.longitudeddff);
                LongLatidata.longitudeddtr[10] = 0;
                for (i = 1; i < 10; i++) {
                    LongLatidata.Truelongitude[1 + i] = LongLatidata.longitudeddtr[i];
                }
                //	sprintf(LongLatidata.data_len,"%d",strlen(LongLatidata.buffer)/sizeof(char));//长度转成字符串
                //		EC200S_TCPSend((u8*)LongLatidata.buffer);
            }
            memset(LongLatidata.buffer, 0, 200);

            //  OLED_ShowString(18,0,"mzhLon:",8);
            OLED_ShowString(22, 1, LongLatidata.TrueLatitude, 6);
            // OLED_ShowString(18,3,"mzhLat:",8);
            OLED_ShowString(22, 5, LongLatidata.Truelongitude, 6);
            if (LongLatidata.RTKflag == '2')
                OLED_ShowString(25, 6, "DIFF MDOE", 6);
            else if (LongLatidata.RTKflag == '5')
                OLED_ShowString(25, 6, "FLOAT RTK", 6);
            else if (LongLatidata.RTKflag == '4')
                OLED_ShowString(25, 6, "FIXED RTK", 12);
        } else {
            OLED_ShowString(25, 1, "000.000000", 6);
            OLED_ShowString(25, 4, "00.000000", 6);
            //	EC200S_TCPSend((u8*)"0000.00000");

        }

    }
}


void Getdata_uwb(void) {
    char *rest; // 用于保存 strtok_s 的上下文
    unsigned char i;
    char *strx;
    char *p;
    char json[] = "{lon:%d.%06d:lat:%d.%06d}";
    memset(uwbdata.uwb_rawdata, 0, 100);
    strx = strstr((const char *) Uart4_RxBuff, (const char *) "x");//
    if (strx) {
        uwbdata.uwb_flag = 1;

        memcpy(uwbdata.uwb_rawdata, Uart4_RxBuff, sizeof(Uart4_RxBuff));
        memset(uwbdata.uwb_x, 0, 10);
        memset(uwbdata.uwb_y, 0, 10);
        memset(uwbdata.uwb_z, 0, 10);
//        memset(uwbdata.TrueLatitude, 0, 50);
        p = strtok_r(uwbdata.uwb_rawdata, ",", &rest);
        p = strtok_r(NULL, ",", &rest);
        memcpy(uwbdata.uwb_x, p, 10);
        p = strtok_r(NULL, ",", &rest);
        p = strtok_r(NULL, ",", &rest);
        memcpy(uwbdata.uwb_y, p, 10);
        p = strtok_r(NULL, ",", &rest);
        p = strtok_r(NULL, ",", &rest);
        memcpy(uwbdata.uwb_z, p, 10);
        p = strtok_r(NULL, ",", &rest);
        p = strtok_r(NULL, ",", &rest);
        memcpy(uwbdata.uwb_strength, p, 10);
//        memcpy(uwbdata., p, 11);

    }
}

// 初始化滤波器
void init_filter(DOPFilter *filter) {
    for(int i=0; i<WINDOW_SIZE; i++) {
        filter->hdop_history[i] = 99.9;  // 用极大值初始化表示无效数据
        filter->pdop_history[i] = 99.9;
    }
    filter->index = 0;
}
// 更新滤波器数据
void update_filter(DOPFilter *filter, float hdop, float pdop) {
    // 写入当前数据
    filter->hdop_history[filter->index] = hdop;
    filter->pdop_history[filter->index] = pdop;

    // 循环移动索引
    filter->index = (filter->index + 1) % WINDOW_SIZE;
}

// 判断是否在室外（带窗口滤波）
bool is_outdoor(DOPFilter *filter) {
    float hdop_sum = 0, pdop_sum = 0;
    int valid_count = 0;

    // 计算窗口内有效数据的平均值
    for(int i=0; i<WINDOW_SIZE; i++) {
        if(filter->hdop_history[i] < 50 && filter->pdop_history[i] < 50) {
            hdop_sum += filter->hdop_history[i];
            pdop_sum += filter->pdop_history[i];
            valid_count++;
        }
    }

    // 需要至少3个有效数据点才进行判断
    if(valid_count < 3) return false;

    float avg_hdop = hdop_sum / valid_count;
    float avg_pdop = pdop_sum / valid_count;

    // 判断条件：HDOP < 2.5 且 PDOP < 5.0
    return (avg_hdop < 2.5f && avg_pdop < 5.0f);
}




void OLED_SHOWAHT20(void) {

    OLED_ShowString(18, 0, "mzhLon:", 8);
    OLED_ShowString(18, 2, "4g", 8);
    OLED_ShowString(18, 4, "mzhLat:", 8);
    OLED_ShowString(18, 7, "lc29", 8);

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {


    if (huart == &huart3) {


//        memset(rx_buffer, 0, BUFFER_SIZE);     // 清空缓存（按需选择）
// 解锁 UART 句柄
//        __HAL_UNLOCK(huart);
// 重新启动 UART 接收中断
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, G_Uart3_RxBuff, sizeof(G_Uart3_RxBuff));
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
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, G_Uart2_RxBuff, sizeof(G_Uart2_RxBuff));

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
