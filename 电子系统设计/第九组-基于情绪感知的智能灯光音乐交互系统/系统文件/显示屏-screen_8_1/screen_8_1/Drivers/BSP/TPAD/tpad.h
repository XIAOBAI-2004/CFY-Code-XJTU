/**
 ****************************************************************************************************
 * @file        tpad.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2020-04-21
 * @brief       ���ݴ�������(TPAD) ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20200421
 * ��һ�η���
 *
 * V1.1 20221101
 * 1, �޸�tpad_scan, ����ۺ�ʵ��, ��ǿ���������� 
 * 2, �޸�tpad_reset, ��ǰ�ŵ�, ��ǿ����������
 *
 ****************************************************************************************************
 */

#ifndef __TPAD_H
#define __TPAD_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* TPAD ���� �� ��ʱ�� ���� */

/* ����ʹ�ö�ʱ�������벶����, ��TPAD���м��
 * ��������벶��ʹ�ö�ʱ��TIM2_CH1, ����TPAD����������
 */
#define TPAD_GPIO_PORT                          GPIOA
#define TPAD_GPIO_PIN                           SYS_GPIO_PIN5
#define TPAD_GPIO_AF                            1                                               /* AF����ѡ�� */
#define TPAD_GPIO_CLK_ENABLE()                  do{ RCC->AHB1ENR |= 1 << 0; }while(0)           /* PA��ʱ��ʹ�� */

#define TPAD_TIMX_CAP                           TIM2
#define TPAD_TIMX_CAP_CHY                       1                                               /* ͨ��Y,  1<= Y <=4 */
#define TPAD_TIMX_CAP_CHY_CCRX                  TIM2->CCR1                                      /* ͨ��Y�Ĳ���/�ȽϼĴ��� */
#define TPAD_TIMX_CAP_CHY_CLK_ENABLE()          do{ RCC->APB1ENR |= 1 << 0; }while(0)           /* TIM2 ʱ��ʹ�� */

/******************************************************************************************/

/* ����������ֵ, Ҳ���Ǳ������ g_tpad_default_val + TPAD_GATE_VAL
 * ����Ϊ����Ч����, �Ĵ� TPAD_GATE_VAL, ���Խ���������, ��֮, ��������������
 * ����ʵ������, ѡ����ʵ� TPAD_GATE_VAL ����
 */
#define TPAD_GATE_VAL       100                 /* ����������ֵ, Ҳ���Ǳ������ g_tpad_default_val + TPAD_GATE_VAL, ����Ϊ����Ч���� */
#define TPAD_ARR_MAX_VAL    0xFFFF              /* ����ARRֵ, һ������Ϊ��ʱ����ARR���ֵ */


/* �ӿں���, ����������.c���� */
uint8_t tpad_init(uint16_t psc);                /* TPAD ��ʼ�� ���� */
uint8_t tpad_scan(uint8_t mode);                /* TPAD ɨ�� ���� */


extern volatile uint16_t g_tpad_default_val;    /* ���ص�ʱ��(û���ְ���),��������Ҫ��ʱ�� */

/* ��̬����, ���� tapd.c���� */
static void tpad_reset(void);                   /* ��λ */
static uint16_t tpad_get_val(void);             /* �õ���ʱ������ֵ */
static uint16_t tpad_get_maxval(uint8_t n);     /* ��ȡn��, ��ȡ���ֵ */
static void tpad_timx_cap_init(uint16_t arr, uint16_t psc); /* ��ʱ�����벶���ʼ�� */


#endif













