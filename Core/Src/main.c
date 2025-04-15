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
uint8_t aRxBuffer1, aRxBuffer2, aRxBuffer3, aRxBuffer4;            //�������ݴ洢����
uint8_t Uart1_RxBuff[100];        //��������
uint8_t Uart1_Rx_Cnt = 0;        //����
uint8_t Uart2_RxBuff[2000] = {0};        //��������
uint8_t Uart2_Rx_Cnt = 0;        //����
uint8_t Uart3_RxBuff[2000] = {0};        //��������
uint8_t Uart3_Rx_Cnt = 0;        //����
uint8_t Uart4_RxBuff[100];        //��������
uint8_t Uart4_Rx_Cnt = 0;        //����
//uint8_t Command_Handle_Buff[2000] = {0};        //��������


static uint8_t main_buf[2000];


int num_4g = 0;
int num_lc29 = 0;


extern unsigned char uart1_getok, uart2_getok, uart3_getok;

struct {
    uint8_t Receive_End;
    uint8_t Receive_Start;
    uint8_t Receive_Count;
    uint16_t Receive_last_length;
    uint8_t uart3_getok;
} Receive_Handle;
extern unsigned int DMArx_len;
//GPSģ��ľ�γ������ֵ
struct {
    char Latitude[20];//����ԭ����
    char longitude[20];//γ��Դ����
    char Latitudess[3];//��������
    char longitudess[2];
    char Latitudedd[10];//С���㲿��
    char longitudedd[10];
    float Latitudeddff;
    float longitudeddff;
    char Latitudeddtr[12];//С���㲿��
    char longitudeddtr[12];
    char TrueLatitude[50];//ת��������
    char Truelongitude[50];//ת��������
    char buffer[200];
    char RTKflag;
    char GGAdata[100];
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



    //HAL_UART_Receive_DMA(&huart2, Uart2_RxBuff, 200);

    int num = 0;
    int num2 = 0;

    char buf[3];
    char buf2[12] = {0};
    bool send_mirror = true;

    uint8_t send_data[] = {0};
    uint8_t output_data[] = {0};
    uint8_t send_Lati_data[200];

//    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, 2000);
//    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, 2000);
//
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, Uart4_RxBuff, sizeof(Uart4_RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, sizeof(Uart3_RxBuff));

// ʹ��Ex���������ղ���������
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));




// �ر�DMA��������жϣ�HAL��Ĭ�Ͽ�����������ֻ��Ҫ��������жϣ�
    // __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

    //HAL_UART_Receive_DMA();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

// Ԥ������������
//        const int num_packets = (sizeof (send_data) + 199) / 200;
//        SerialPacket packets[num_packets];
//        Command_Send_Data(send_data,sizeof (send_data),packets);
//        HAL_UART_Transmit_DMA(&huart2, packets, sizeof (packets));

//        ring_buff_v1(command);


        if (Receive_Handle.Receive_End == 1) {
//            uint16_t lenght = 200 *( Receive_Handle.Receive_Count-1) + Receive_Handle.Receive_last_length+5;
            uint16_t lenght = ringbuff_getdata_all(&my_ringbuff, main_buf);

            Command_Analysis_Data(main_buf, lenght, Receive_Handle.Receive_last_length);
//            HAL_UART_Transmit(&huart1, main_buf, lenght,500);
//            HAL_Delay(50);
//
//            HAL_UART_Transmit(&huart3, main_buf, lenght, 500);


//��վ���к���
//            master(send_mirror);

            send_mirror = !send_mirror;
            Receive_Handle.Receive_End = 0;
            Receive_Handle.Receive_Count = 0;
            Receive_Handle.Receive_last_length = 0;

        }


        if (Receive_Handle.uart3_getok)//gps data is ok
        {

            {
                //		memcpy(LongLatidata.ALLNEMAdata,Uart3_RxBuff,Uart3_Rx_Cnt);
                //LongLatidata.ALLNEMAdata[Uart3_Rx_Cnt]=0;
//                if (HAL_UART_Transmit(&huart1, (uint8_t *) Uart3_RxBuff, DMArx_len) != HAL_OK)//DMA SEND
//                {
//                    Error_Handler();
//                }
                Receive_Handle.uart3_getok = 0;
                Getdata_Change();

                memcpy(send_Lati_data,LongLatidata.TrueLatitude,50);
                memcpy(send_Lati_data+50,LongLatidata.Truelongitude,50);

                //  Uart3_Rx_Cnt=0;
//                //	memset(Uart3_RxBuff,0,2000);
//                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
//                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
            }

        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        num++;
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

//void master(bool send_mirror) {
//
////            printf("#################   ��ȡ  ��ȡ ��ʼ ��ȡ  ��ȡ #####################\n");
////            ringbuff_debug(&my_ringbuff);
//    uint16_t lenght = ringbuff_getdata_all(&my_ringbuff, main_buf);
////            printf("���� %d ###\n",lenght);
////            ringbuff_debug(&my_ringbuff);
////            printf("#################   ��ȡ  ��ȡ ���� ��ȡ  ��ȡ #####################\n");
//
//    if (send_mirror) {//            printf(main_buf);
//        int num_packets = ((lenght) + 194) / 195;
////                SerialPacket packets[num_packets];
//        Command_Send_Data(main_buf, lenght, 200);
////                Command_Send_Data_t(main_buf, num_packets, lenght);
//
//        HAL_UART_Transmit(&huart1, main_buf, lenght, 500);
//
//    }
//}


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



//���ڿ����жϻص�����
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    char buf[3];
    char buf2[3];
    //����2���ջص�������LORA�ش������Ϣ ��Ҫ�����ݽ��н����
    if (huart == &huart2) {

//        uint8_t length = CommandBuffer_Write(Uart2_RxBuff, Size);
        ringbuff_putdata(&my_ringbuff, Uart2_RxBuff, Size);

//        ����յ��˽���λ����ʼ��������
        if (Uart2_RxBuff[1] == 0x04) {
            //���ݳ��ȴ���
            Receive_Handle.Receive_Count = Uart2_RxBuff[2];
            Receive_Handle.Receive_last_length = Uart2_RxBuff[4];
            Receive_Handle.Receive_End = 1;
        }
//            HAL_UART_Transmit(&huart1, main_buf, Size,500);

//        num_4g++;
//        sprintf(buf, "%d", num_4g);
//        OLED_ShowString(55, 2, (u8 *) buf, sizeof(buf));

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

    }
    if (huart == &huart3)
        if (Size != 0) {
            {
                //uint8_t length = CommandBuffer_Write(Uart2_RxBuff, Size);
//                printf("#################   д��  д�� ��ʼ д��  д�� #####################\n");
//                ringbuff_debug(&my_ringbuff);
//                printf("д�� %hu\n",Size);
//                ringbuff_putdata(&my_ringbuff, Uart3_RxBuff, Size);
//                ringbuff_debug(&my_ringbuff);
//                printf("#################   д��  д�� ���� д��  д�� #####################\n");
                Receive_Handle.uart3_getok = 1;
//                Receive_Handle.Receive_End = 1;
//        printf(Uart3_RxBuff);
//        HAL_UART_Transmit_DMA(&huart2, Uart3_RxBuff, Size);
//        send_data(uint8_t *input, int Size, SerialPacket *output);
//
//        send_data(uint8_t *data, uint32_t len)


//                num_lc29++;
//                sprintf(buf, "%d", num_lc29);
//                OLED_ShowString(55, 7, (u8 *) buf, sizeof(buf));

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

// ���������ݽ�����ɻص�����
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//    if (huart->Instance == USART2)
//    {
//        // ʹ��DMA�����յ������ݷ��ͻ�ȥ
//        HAL_UART_Transmit_DMA(&huart2, Uart2_RxBuff, Size);
//        // �����������գ�ʹ��Ex���������ղ���������
//        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));
//        // �ر�DMA��������жϣ�HAL��Ĭ�Ͽ�����������ֻ��Ҫ��������жϣ�
//        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
//    }
//}
/*************��ԭʼ���ݽ�������γ������*******************/
//$GNGGA,015032.000,3150.303376,N,11707.855089,E,1,22,1.91,197.8,M,-0.3,M,,*6B
void Getdata_Change(void) {
    unsigned char i;
    char *strx;
    char *p;
    char json[] = "{lon:%d.%06d:lat:%d.%06d}";
    memset(LongLatidata.GGAdata, 0, 100);
    strx = strstr((const char *) Uart3_RxBuff, (const char *) "$GNGGA");//����$GNGGA
    if (strx) {
        for (i = 0;; i++) {
            if (strx[i + 1] == '$')
                break;
            LongLatidata.GGAdata[i] = strx[i];
        }
        LongLatidata.GGAdata[i] = 0;
//        printf(LongLatidata.GGAdata);//mqtt send
        strx = strstr((const char *) LongLatidata.GGAdata, (const char *) "N,");//����N��������γ�����ݱ���ȷ��ȡ��
        if (strx) {

            memcpy(LongLatidata.buffer, LongLatidata.GGAdata, 100);
            memset(LongLatidata.longitude, 0, 20);
            memset(LongLatidata.Latitude, 0, 20);
            memset(LongLatidata.Truelongitude, 0, 50);
            memset(LongLatidata.TrueLatitude, 0, 50);
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
            //printf(LongLatidata.Latitude);
            //	Uart1_SendStr(p);
            strx = strstr((const char *) LongLatidata.GGAdata, (const char *) "E,");//����E����ȡγ������11702.5641
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
                //	sprintf(LongLatidata.data_len,"%d",strlen(LongLatidata.buffer)/sizeof(char));//����ת���ַ���
                //		EC200S_TCPSend((u8*)LongLatidata.buffer);
            }
            memset(LongLatidata.buffer, 0, 200);

            //  OLED_ShowString(18,0,"mzhLon:",8);
            OLED_ShowString(22, 1, LongLatidata.TrueLatitude, 12);
            // OLED_ShowString(18,3,"mzhLat:",8);
            OLED_ShowString(22, 5, LongLatidata.Truelongitude, 12);
            if (LongLatidata.RTKflag == '2')
                OLED_ShowString(25, 6, "DIFF MDOE", 12);
            else if (LongLatidata.RTKflag == '5')
                OLED_ShowString(25, 6, "FLOAT RTK", 12);
            else if (LongLatidata.RTKflag == '4')
                OLED_ShowString(25, 6, "FIXED RTK", 12);
        } else {
            OLED_ShowString(25, 1, "000.000000", 12);
            OLED_ShowString(25, 4, "00.000000", 12);
            //	EC200S_TCPSend((u8*)"0000.00000");

        }

    }
}

void OLED_SHOWAHT20(void) {

    OLED_ShowString(18, 0, "mzhLon:", 8);
    OLED_ShowString(18, 2, "4g", 8);
    OLED_ShowString(18, 4, "mzhLat:", 8);
    OLED_ShowString(18, 7, "lc29", 8);

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {


    if (huart == &huart3) {


//        memset(rx_buffer, 0, BUFFER_SIZE);     // ��ջ��棨����ѡ��
// ���� UART ���
//        __HAL_UNLOCK(huart);
// �������� UART �����ж�
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3_RxBuff, sizeof(Uart3_RxBuff));
        OLED_ShowString(80, 2, "er3", 8);

    }
    if (huart == &huart1) {
        OLED_ShowString(80, 2, "er1", 8);

// ���� UART ���
//        __HAL_UNLOCK(huart);
// �������� UART �����ж�
//        HAL_UART_Receive_IT(&huart1, Tx_M_Buff_Temp[0], 1);
    }
    if (huart == &huart2) {
// ���� UART ���
//        __HAL_UNLOCK(huart);
        OLED_ShowString(80, 2, "er2", 8);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2_RxBuff, sizeof(Uart2_RxBuff));

// �������� UART �����ж�
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
