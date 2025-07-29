/**
 ****************************************************************************************************
 * @file        btim.c
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

#include "./BSP/LED/led.h"
#include "./BSP/TIMER/btim.h"
#include "./SYSTEM/usart/usart.h"

/* 视频播放帧率控制全局变量 */
uint16_t g_avi_frame;                /* 播放帧率 */
volatile uint8_t g_avi_frameup;      /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */

/**
 * @brief       基本定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIMX_INT_IRQHandler(void)
{ 
    if (BTIM_TIMX_INT->SR & 0X0001)     /* 溢出中断 */
    {
        printf("frame:%dfps\r\n", g_avi_frame);  /* 打印帧率 */
        g_avi_frame = 0;
    }

    BTIM_TIMX_INT->SR &= ~(1 << 0);     /* 清除中断标志位 */
} 

/**
 * @brief       基本定时器TIMX定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为42M, 所以定时器时钟 = 84Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
    BTIM_TIMX_INT_CLK_ENABLE();
    BTIM_TIMX_INT->ARR = arr;           /* 设定计数器自动重装值 */
    BTIM_TIMX_INT->PSC = psc;           /* 设置预分频器  */
    BTIM_TIMX_INT->DIER |= 1 << 0;      /* 允许更新中断 */
    BTIM_TIMX_INT->CR1 |= 1 << 0;       /* 使能定时器TIMX */
    sys_nvic_init(1, 3, BTIM_TIMX_INT_IRQn, 2); /* 抢占1，子优先级3，组2 */
}

/**
 * @brief       基本定时器TIM7中断服务函数
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{
    if (TIM7->SR & 0X0001)      /* 溢出中断 */
    {
        g_avi_frameup = 1;      /* 标记时间到 */
    }

    TIM7->SR &= ~(1 << 0);      /* 清除中断标志位 */
}

/**
 * @brief       基本定时器TIM7定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为42M, 所以定时器时钟 = 84Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim7_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1ENR |= 1 << 5;     /* 使能TIM7时钟 */
    TIM7->ARR = arr;            /* 设定计数器自动重装值 */
    TIM7->PSC = psc;            /* 设置预分频器  */
    TIM7->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM7->CR1 |= 1 << 0;        /* 使能定时器TIMX */
    sys_nvic_init(0, 3, TIM7_IRQn, 2);  /* 抢占0，子优先级3，组2 */
}






