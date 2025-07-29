/**
 ****************************************************************************************************
 * @file        usart3.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       串口3 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20221202
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __USART3_H
#define __USART3_H

#include "./SYSTEM/sys/sys.h"
#include "stdio.h"
#include "ucos_ii.h"

/******************************************************************************************/
/* USART3 引脚 定义
 */
#define USART3_TX_GPIO_PORT                 GPIOB
#define USART3_TX_GPIO_PIN                  SYS_GPIO_PIN10
#define USART3_TX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB口时钟使能 */

#define USART3_RX_GPIO_PORT                 GPIOB
#define USART3_RX_GPIO_PIN                  SYS_GPIO_PIN11
#define USART3_RX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB口时钟使能 */

/******************************************************************************************/

#define USART3_MAX_RECV_LEN     600         /* 最大接收缓存字节数 */
#define USART3_MAX_SEND_LEN     600         /* 最大发送缓存字节数 */
#define USART3_RX_EN            1           /* 0,不接收;1,接收 */


extern uint8_t  g_usart3_rx_buf[USART3_MAX_RECV_LEN];   /* 接收缓冲,最大USART3_MAX_RECV_LEN字节 */
extern volatile uint16_t g_usart3_rx_sta;                 /* 接收数据状态 */


void usart3_init(uint32_t sclk,uint32_t baudrate);
void u3_printf(char* fmt,...);



volatile extern uint8_t Serial_RxFlag;
extern volatile char Serial_RxPacket[100];		
extern const char *cmd_str[];

#endif	   
















