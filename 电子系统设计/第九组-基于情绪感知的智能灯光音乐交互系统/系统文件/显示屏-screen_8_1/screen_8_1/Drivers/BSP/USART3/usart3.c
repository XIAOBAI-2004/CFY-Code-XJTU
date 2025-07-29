/****************************************************************************************************
 * @file        usart3.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-10-29
 * @brief       ����3 ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************/
#include "./BSP/USART3/usart3.h"
#include "./BSP/TIMER/timer.h"
#include "./MALLOC/malloc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ucos_ii.h"

#include "os.h"   /* os ʹ�� */
#include "audioplay.h"

/* ���ʹ��os,����������ͷ�ļ�����. */
#if SYS_SUPPORT_OS
#include "os.h"   /* os ʹ�� */
#endif

/******************************************************************************************/
/* �������´���, ֧��printf����, ������Ҫѡ��use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)            /* ʹ��AC6������ʱ */
__asm(".global __use_no_semihosting\n\t");  /* ������ʹ�ð�����ģʽ */
__asm(".global __ARM_use_no_argv \n\t");    /* AC6����Ҫ����main����Ϊ�޲�����ʽ�����򲿷����̿��ܳ��ְ�����ģʽ */

#else
/* ʹ��AC5������ʱ, Ҫ�����ﶨ��__FILE �� ��ʹ�ð�����ģʽ */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* ��ʹ�ð�����ģʽ��������Ҫ�ض���_ttywrch_sys_exit_sys_command_string����,��ͬʱ����AC6��AC5ģʽ */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE �� stdio.h���涨��. */
FILE __stdout;

/* �ض���fputc����, printf�������ջ�ͨ������fputc����ַ��������� */
//volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
//volatile uint8_t tx_head = 0;
//volatile uint8_t tx_tail = 0;

//void usart3_send_char(uint8_t ch)
//{
//    while (((tx_head + 1) % TX_BUFFER_SIZE) == tx_tail);  // �ȴ��������пռ�
//    tx_buffer[tx_head] = ch;
//    tx_head = (tx_head + 1) % TX_BUFFER_SIZE;
//}

//void usart3_flush(void)
//{
//    if (tx_head != tx_tail)
//    {
//        USART3->DR = tx_buffer[tx_tail];
//        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
//    }
//}

int fputc(int ch, FILE *f)
{
    while ((USART3->SR & USART_SR_TXE) == 0);     /* �ȴ���һ���ַ�������� */
    USART3->DR = (uint8_t)ch;                     /* ��Ҫ���͵��ַ� ch д�뵽DR�Ĵ��� */
    return ch;
}
#endif
/******************************************************************************************/

/* ���ڽ��ջ����� */
uint8_t g_usart3_rx_buf[USART3_MAX_RECV_LEN];       /* ���ջ���,���USART3_MAX_RECV_LEN���ֽ� */
volatile uint16_t g_usart3_rx_sta = 0;

volatile uint8_t Serial_RxFlag = 0;
volatile char Serial_RxPacket[100];				
/**
 * @brief       ����3�жϷ�����
 * @param       ��
 * @retval      ��
 */


// �����ַ������飬˳�������ö��һ��
const char *cmd_str[] = {
    "bofang",    // ����
    "zanting",   // ��ͣ
    "shangyishou", // ��һ��
    "xiayishou" // ��һ��
//	"liushui",    // ��ˮ
//	"qisejianbian", // ��ɫ����
//	"shanshuo",    //  ��˸
//	"xunhuan",    //   ѭ��	
//	"kaideng",    //����
//	"guandeng",    //�ص�
//	"kaixin",     //   ����
//	"fennu",      //  ��ŭ
//	"beishang",   //   ����
//	"kuaile",     //   ����
//	"fuza"       //   ����
};

extern OS_FLAG_GRP *voice_event; // �¼����ƿ�

void USART3_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint16_t pRxPacket = 0;
    uint8_t RxData;
    INT8U err;
    static voice_cmd_t static_cmd;  // ʹ�þ�̬�������⶯̬����

    OSIntEnter();
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();

    if (USART3->SR & (1 << 5)) {
        RxData = USART3->DR;

        switch (RxState) {
            case 0: // �ȴ���ͷ'@'
                if (RxData == '@' && Serial_RxFlag == 0) {
                    RxState = 1;
                    pRxPacket = 0;
                    g_usart3_rx_sta = 0;
									  memset((void*)Serial_RxPacket, 0, USART3_MAX_RECV_LEN); // �������ת��TIM7->CNT = 0;
                    TIM7->CR1 |= (1 << 0);
                }
                break;

            case 1: // ������������
                if (RxData == '%') {
                    RxState = 2; // ����ȴ���β״̬
                } else {
                    if (pRxPacket < USART3_MAX_RECV_LEN - 1) {
                        Serial_RxPacket[pRxPacket++] = RxData;
                    } else {
                        RxState = 0;
                        TIM7->CR1 &= ~(1 << 0);
                        g_usart3_rx_sta |= (1 << 15);
                    }
                    TIM7->CNT = 0;
                }
                break;

            case 2: // ��֤��β'\n'
                if (RxData == '#') {
                    Serial_RxPacket[pRxPacket] = '\0'; // ��ȷ�����ַ���
									  
									u3_printf("Received full packet: %s\r\n", Serial_RxPacket);
									/* ������ָ���������� */
         /* ������ָ���������� */
        if (strncmp((const char *)Serial_RxPacket, "SONG:", 5) == 0) {
            voice_cmd_ex_t *cmd = mymalloc(SRAMIN, sizeof(voice_cmd_ex_t));
            if(cmd) {
                cmd->cmd_type = VOICE_CMD_PLAY_SONG;
                uint8_t *song_start = Serial_RxPacket + 5;
                
                /* ������ܵĻ���/�س��� */
                uint8_t *p = song_start;
                while(*p && *p != '\r' && *p != '\n') p++;
                *p = '\0';
                
                /* ֱ�Ӵ洢UTF-8�ַ��� */
                uint8_t len = strlen((const char*)song_start);
                len = (len > MAX_SONG_NAME_LEN-1) ? MAX_SONG_NAME_LEN-1 : len;
                memcpy(cmd->song_name, song_start, len);
                cmd->song_name[len] = '\0';
                
                OSQPost(g_voice_q, cmd);
                OSFlagPost(voice_event, 0x01, OS_FLAG_SET, &err);
            }
        }
        else {
									
									
                    for (int i = 0; i < sizeof(cmd_str)/sizeof(char*); i++) {
                        if (strstr((char *)Serial_RxPacket, cmd_str[i])) {
													  
                            static_cmd = (voice_cmd_t)(i+1);  // ʹ�þ�̬����
                            OSQPostOpt(g_voice_q, &static_cmd, OS_POST_OPT_FRONT);
                            OSFlagPost(voice_event, 0x01, OS_FLAG_SET, &err);
                            Serial_RxFlag = 1;  // ��ȷ���ñ�־
													  
                            break;
                        }
                    }
                } 
//				else {
//                    // ��β���󣬶�������
//                    g_usart3_rx_sta |= (1 << 15);
//                }
							}
                RxState = 0;
                pRxPacket = 0;
                TIM7->CR1 &= ~(1 << 0);
                break;
        }

        USART3->SR &= ~(1 << 5);
    }

    OSIntExit();
    OS_EXIT_CRITICAL();
}
	

/**
 * @brief       ����3��ʼ������
 * @param       sclk: ����X��ʱ��ԴƵ��(��λ: MHz)
 *              ����1 ��ʱ��Դ����: PCLK2 = 72Mhz
 *              ����2 - 5 ��ʱ��Դ����: PCLK1 = 36Mhz
 * @note        ע��: ����������ȷ��sclk, ���򴮿ڲ����ʾͻ������쳣.
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @retval      ��
 */


void usart3_init(uint32_t sclk, uint32_t baudrate)
{
    USART3_TX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���TX��ʱ�� */
    USART3_RX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���RX��ʱ�� */
    
   /* TX���ţ�PB10������Ϊ����������� */
    sys_gpio_set(USART3_TX_GPIO_PORT, USART3_TX_GPIO_PIN,
                SYS_GPIO_MODE_AF,          // ����ģʽ
                SYS_GPIO_OTYPE_PP,         // �������
                SYS_GPIO_SPEED_HIGH,       // ����
                SYS_GPIO_PUPD_PU);         // ����

    /* RX���ţ�PB11������Ϊ�������� */
    sys_gpio_set(USART3_RX_GPIO_PORT, USART3_RX_GPIO_PIN,
                SYS_GPIO_MODE_AF,          // ����ģʽ
                SYS_GPIO_OTYPE_PP,
                SYS_GPIO_SPEED_HIGH,
                SYS_GPIO_PUPD_PU);
	
    sys_gpio_af_set(USART3_TX_GPIO_PORT, USART3_TX_GPIO_PIN, 7);    /* TX�� ���ù���ѡ��, ����������ȷ */
    sys_gpio_af_set(USART3_RX_GPIO_PORT, USART3_RX_GPIO_PIN, 7);    /* RX�� ���ù���ѡ��, ����������ȷ */

    RCC->APB1ENR |= 1 << 18;        /* ʹ�ܴ���3ʱ�� */
    RCC->APB1RSTR |= 1 << 18;       /* ��λ����3 */
    RCC->APB1RSTR &= ~(1 << 18);    /* ֹͣ��λ */
    
    /* ���������� */
    USART3->BRR = (sclk * 1000000) / (baudrate);
    USART3->CR1 |= 0X200C;          /* 1λֹͣ,��У��λ */
    
    /* ʹ�ܽ����ж� */
    USART3->CR1 |= 1 << 5;          /* ���ջ������ǿ��ж�ʹ�� */
    
    sys_nvic_init(0, 1, USART3_IRQn, 2); /* ��2 */
   
    tim7_int_init(99, 8400 - 1);    /* 10ms�ж� */
    TIM7->CR1 &= ~(1 << 0);         /* �رն�ʱ��7 */
    g_usart3_rx_sta = 0;            /* ���� */
}

/**
 * @brief       ����3,printf ����
 * @param       fmt     : ��ʽ������
 * @param       ...     : �ɱ����
 * @retval      ��
 */
void u3_printf(char *fmt, ...)
{
    uint16_t i, j;
    uint8_t *pbuf;
    va_list ap;
    
    pbuf = mymalloc(SRAMIN, USART3_MAX_SEND_LEN);   /* �����ڴ� */

    if (!pbuf)  /* �ڴ�����ʧ�� */
    {
        printf("u3 malloc error\r\n");
        return ;
    }

    va_start(ap, fmt);
    vsprintf((char *)pbuf, fmt, ap);
    va_end(ap);
    i = strlen((const char *)pbuf); /* �˴η������ݵĳ��� */

    for (j = 0; j < i; j++)         /* ѭ���������� */
    {
        while ((USART3->SR & 0X40) == 0);   /* ѭ������,ֱ��������� */

        USART3->DR = pbuf[j];
    }

    myfree(SRAMIN, pbuf);           /* �ͷ��ڴ� */
}

