/**
 ****************************************************************************************************
 * @file        btim.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.2
 * @date        2022-01-18
 * @brief       基本定时器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20211230
 * 第一次发布
 * V1.1 20220113
 * 1, 修改BTIM_TIMX_INT_IRQHandler, 以支持OV2640驱动
 * 2, 新增 g_ov2640_frame 全局变量, 用于统计帧率
 * V1.2 20220118
 * 1, 新增 btim_tim7_int_init 函数
 *
 ****************************************************************************************************
 */

#ifndef __BTIM_H
#define __BTIM_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* 基本定时器 定义 */

/* TIMX 中断定义 
 * 默认是针对TIM6/TIM7
 * 注意: 通过修改这4个宏定义,可以支持TIM1~TIM14任意一个定时器.
 */
 
#define BTIM_TIMX_INT                       TIM6
#define BTIM_TIMX_INT_IRQn                  TIM6_DAC_IRQn
#define BTIM_TIMX_INT_IRQHandler            TIM6_DAC_IRQHandler
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ RCC->APB1ENR |= 1 << 4; }while(0)  /* TIM6 时钟使能 */

/******************************************************************************************/

void btim_timx_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim7_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器7 定时中断初始化函数 */

#endif

















