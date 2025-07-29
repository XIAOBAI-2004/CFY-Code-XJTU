/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       ����3 ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20221202
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

#include "./BSP/TIMER/timer.h"
#include "./MALLOC/malloc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ucos_ii.h"

#include "./BSP/USART3/usart3.h"

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

/* ��ʹ�ð�����ģʽ��������Ҫ�ض���_ttywrch\_sys_exit\_sys_command_string����,��ͬʱ����AC6��AC5ģʽ */

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */


/* FILE �� stdio.h���涨��. */


/* �ض���fputc����, printf�������ջ�ͨ������fputc����ַ��������� */

#endif
/******************************************************************************************/


#if USART_EN_RX     /* ���ʹ���˽��� */

/* ���ջ���, ���USART_REC_LEN���ֽ�. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  ����״̬
 *  bit15��      ������ɱ�־
 *  bit14��      ���յ�0x0d
 *  bit13~0��    ���յ�����Ч�ֽ���Ŀ
*/
uint16_t g_usart_rx_sta = 0;

/**
 * @brief       ����X�жϷ�����
 * @param       ��
 * @retval      ��
 */
 
 
extern volatile uint8_t Serial_RxFlag;
extern volatile char Serial_RxPacket[100];		




// �����ַ������飬˳�������ö��һ��
// �����ַ������飬˳�������ö��һ��
const char *cmd_str1[] = {
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



/**
 * @brief       ����3�жϷ�����
 * @param       ��
 * @retval      ��
 */




extern OS_FLAG_GRP *voice_event; // �¼����ƿ�

void USART_UX_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint16_t pRxPacket = 0;
    uint8_t RxData;
    INT8U err;
    static voice_cmd_t static_cmd;  // ʹ�þ�̬�������⶯̬����

    OSIntEnter();
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
	
    if (USART_UX->SR & (1 << 5)) {
        RxData = USART_UX->DR;

        switch (RxState) {
            case 0: // �ȴ���ͷ'@'
                if (RxData == '@' && Serial_RxFlag == 0) {
                    RxState = 1;
                    pRxPacket = 0;
                    g_usart_rx_sta = 0;
									  memset((void*)Serial_RxPacket, 0, USART_REC_LEN); // �������ת��TIM7->CNT = 0;
                  //  TIM7->CR1 |= (1 << 0);
                }
                break;

            case 1: // ������������
                if (RxData == '%') {
                    RxState = 2; // ����ȴ���β״̬
                } else {
                    if (pRxPacket < USART_REC_LEN - 1) {
                        Serial_RxPacket[pRxPacket++] = RxData;
                    } else {
                        RxState = 0;
                        //TIM7->CR1 &= ~(1 << 0);
                        g_usart_rx_sta |= (1 << 15);
                    }
                   // TIM7->CNT = 0;
                }
                break;

            case 2: // ��֤��β'\n'
                if (RxData == '#') {
                    Serial_RxPacket[pRxPacket] = '\0'; // ��ȷ�����ַ���
									  
									printf("Received full packet: %s\r\n", Serial_RxPacket);
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
									
									
                    for (int i = 0; i < sizeof(cmd_str1)/sizeof(char*); i++) {
                        if (strstr((char *)Serial_RxPacket, cmd_str1[i])) {
													  
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
               // TIM7->CR1 &= ~(1 << 0);
                break;
        }

        USART_UX->SR &= ~(1 << 5);
    }

    OSIntExit();
    OS_EXIT_CRITICAL();
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
//void USART_UX_IRQHandler(void)
//{
//    uint8_t rxdata;
//#if SYS_SUPPORT_OS  /* ���SYS_SUPPORT_OSΪ�棬����Ҫ֧��OS. */
//    OSIntEnter();
//#endif

//    if (USART_UX->SR & (1 << 5))                /* ���յ����� */
//    {
//        rxdata = USART_UX->DR;

//        if ((g_usart_rx_sta & 0x8000) == 0)     /* ����δ���? */
//        {
//            if (g_usart_rx_sta & 0x4000)        /* ���յ���0x0d? */
//            {
//                if (rxdata != 0x0a)             /* ���յ���0x0a? (�����Ƚ��յ���0x0d,�ż��0x0a) */
//                {
//                    g_usart_rx_sta = 0;         /* ���մ���, ���¿�ʼ */
//                }
//                else
//                {
//                    g_usart_rx_sta |= 0x8000;   /* �յ���0x0a,��ǽ�������� */
//                }
//            }
//            else      /* ��û�յ�0x0d */
//            {
//                if (rxdata == 0x0d)
//                {
//                    g_usart_rx_sta |= 0x4000;   /* ��ǽ��յ��� 0x0d */
//                }
//                else
//                {
//                    g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = rxdata;   /* �洢���ݵ� g_usart_rx_buf */
//                    g_usart_rx_sta++;

//                    if (g_usart_rx_sta > (USART_REC_LEN - 1))g_usart_rx_sta = 0;/* �����������, ���¿�ʼ���� */
//                }
//            }
//        }
//    }

//#if SYS_SUPPORT_OS  /* ���SYS_SUPPORT_OSΪ�棬����Ҫ֧��OS. */
//    OSIntExit();
//#endif
//}
#endif

/**
 * @brief       ����X��ʼ������
 * @param       sclk: ����X��ʱ��ԴƵ��(��λ: MHz)
 *              ����1 �� ����6 ��ʱ��Դ����: rcc_pclk2 = 84Mhz
 *              ����2 - 5 / 7 / 8 ��ʱ��Դ����: rcc_pclk1 = 42Mhz
 * @note        ע��: ����������ȷ��sclk, ���򴮿ڲ����ʾͻ������쳣.
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @retval      ��
 */
void usart_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
    /* IO �� ʱ������ */
    USART_TX_GPIO_CLK_ENABLE(); /* ʹ�ܴ���TX��ʱ�� */
    USART_RX_GPIO_CLK_ENABLE(); /* ʹ�ܴ���RX��ʱ�� */
    USART_UX_CLK_ENABLE();      /* ʹ�ܴ���ʱ�� */

    sys_gpio_set(USART_TX_GPIO_PORT, USART_TX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* ����TX�� ģʽ���� */

    sys_gpio_set(USART_RX_GPIO_PORT, USART_RX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* ����RX�� ģʽ���� */

    sys_gpio_af_set(GPIOA, USART_TX_GPIO_PIN, USART_TX_GPIO_AF);    /* TX�� ���ù���ѡ��, ����������ȷ */
    sys_gpio_af_set(GPIOA, USART_RX_GPIO_PIN, USART_RX_GPIO_AF);    /* RX�� ���ù���ѡ��, ����������ȷ */

    temp = (sclk * 1000000 + baudrate / 2) / baudrate;              /* �õ�USARTDIV@OVER8 = 0, ��������������� */
    /* ���������� */
    USART_UX->BRR = temp;       /* ����������@OVER8 = 0 */
    USART_UX->CR1 = 0;          /* ����CR1�Ĵ��� */
    USART_UX->CR1 |= 0 << 12;   /* ����M = 0, ѡ��8λ�ֳ� */
    USART_UX->CR1 |= 0 << 15;   /* ����OVER8 = 0, 16�������� */
    USART_UX->CR1 |= 1 << 3;    /* ���ڷ���ʹ�� */
#if USART_EN_RX  /* ���ʹ���˽��� */
    /* ʹ�ܽ����ж� */
    USART_UX->CR1 |= 1 << 2;    /* ���ڽ���ʹ�� */
    USART_UX->CR1 |= 1 << 5;    /* ���ջ������ǿ��ж�ʹ�� */
    sys_nvic_init(3, 3, USART_UX_IRQn, 2); /* ��2��������ȼ� */
#endif
    USART_UX->CR1 |= 1 << 13;   /* ����ʹ�� */
}














