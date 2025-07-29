/**
 ****************************************************************************************************
 * @file        pwmdac.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       PWM DAC输出 驱动代码
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
 * V1.0 20220111
 * 第一次发布
 *
 ****************************************************************************************************
 */


#ifndef __PWMDAC_H
#define __PWMDAC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* PWM DAC 引脚 和 定时器 定义 */

/* PWMDAC 默认是使用 PA8, 对应的定时器为 TIM1_CH1, 如果你要修改成其他IO输出, 则相应
 * 的定时器及通道也要进行修改. 请根据实际情况进行修改.
 */
#define PWMDAC_GPIO_PORT                    GPIOA
#define PWMDAC_GPIO_PIN                     SYS_GPIO_PIN3
#define PWMDAC_GPIO_AF                      1                                       /* AF功能选择 */
#define PWMDAC_GPIO_CLK_ENABLE()            do{ RCC->AHB1ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */

#define PWMDAC_TIMX                         TIM2
#define PWMDAC_TIMX_CHY                     4                           /* 通道Y,  1<= Y <=4 */
#define PWMDAC_TIMX_CCRX                    PWMDAC_TIMX->CCR4           /* 通道Y的输出比较寄存器 */
#define PWMDAC_TIMX_CLK_ENABLE()            do{ RCC->APB1ENR |= 1 << 0; }while(0)   /* TIM2 时钟使能 */

/******************************************************************************************/

void pwmdac_init(uint16_t arr, uint16_t psc);   /* PWM DAC初始化 */
void pwmdac_set_voltage(uint16_t vol);          /* PWM DAC设置输出电压 */

#endif

















