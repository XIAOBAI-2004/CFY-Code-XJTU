#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "Serial_2.h"   

char Serial2_RxPacket[100];            // �������ݰ�����
uint8_t Serial2_RxFlag;                // �������ݰ���־λ

/**
  * ��    ��������2��ʼ��
  * ��    ������
  * �� �� ֵ����
  */
void Serial2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    // USART2��APB1����
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);     // GPIOAʱ����APB2

    GPIO_InitTypeDef GPIO_InitStructure;
    // TX - PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX - PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;          // �޸��ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

/* ����2���͵��ֽ� */
void Serial2_SendByte(uint8_t Byte)
{
    USART_SendData(USART2, Byte);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

/* ����2�������� */
void Serial2_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i ++) {
        Serial2_SendByte(Array[i]);
    }
}

/* ����2�����ַ��� */
void Serial2_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i ++) {
        Serial2_SendByte(String[i]);
    }
}

/* �η����������ֲ��䣩 */
uint32_t Serial2_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y --) {
        Result *= X;
    }
    return Result;
}

/* ����2�������� */
void Serial2_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i ++) {
        Serial2_SendByte(Number / Serial2_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/* printf�ض��� */
int fputc2(int ch, FILE *f)
{
    Serial2_SendByte(ch);
    return ch;
}

/* ��ʽ����� */
void Serial2_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial2_SendString(String);
}

/* ����2�жϷ����� */
void USART2_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint8_t pRxPacket = 0;
    
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART2);
        
        if (RxState == 0) {
            if (RxData == '@' && Serial2_RxFlag == 0) {
                RxState = 1;
                pRxPacket = 0;
            }
        } else if (RxState == 1) {
            if (RxData == '%') {
                RxState = 2;
            } else {
                Serial2_RxPacket[pRxPacket] = RxData;
                pRxPacket ++;
            }
        } else if (RxState == 2) {
            if (RxData == '#') {
                RxState = 0;
                Serial2_RxPacket[pRxPacket] = '\0';
                Serial2_RxFlag = 1;
            }
        }
        
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
