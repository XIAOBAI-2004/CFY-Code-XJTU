/**
 ****************************************************************************************************
 * @file        i2s.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2022-01-16
 * @brief       I2S 驱动代码
 *              这里使用的是I2S2这个I2S
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
 * V1.0 20220114
 * 第一次发布
 * V1.1 20220116
 * 新增 i2sext_init, i2sext_rx_dma_init, i2s_rec_start和i2s_rec_stop等4个函数
 *
 ****************************************************************************************************
 */

#ifndef __I2S_H
#define __I2S_H
#include "./SYSTEM/SYS/sys.h"


/******************************************************************************************/
/* I2S 引脚 定义 */

#define I2S_LRCK_GPIO_PORT              GPIOB
#define I2S_LRCK_GPIO_PIN               SYS_GPIO_PIN12
#define I2S_LRCK_GPIO_AF                5                                               /* AF功能选择 */
#define I2S_LRCK_GPIO_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 1; }while(0)           /* PB口时钟使能 */

#define I2S_SCLK_GPIO_PORT              GPIOB
#define I2S_SCLK_GPIO_PIN               SYS_GPIO_PIN13
#define I2S_SCLK_GPIO_AF                5                                               /* AF功能选择 */
#define I2S_SCLK_GPIO_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 1; }while(0)           /* PB口时钟使能 */

#define I2S_SDOUT_GPIO_PORT             GPIOC
#define I2S_SDOUT_GPIO_PIN              SYS_GPIO_PIN2
#define I2S_SDOUT_GPIO_AF               6                                               /* AF功能选择,这里I2S2ext_SD必须选6! */
#define I2S_SDOUT_GPIO_CLK_ENABLE()     do{ RCC->AHB1ENR |= 1 << 2; }while(0)           /* PC口时钟使能 */

#define I2S_SDIN_GPIO_PORT              GPIOC
#define I2S_SDIN_GPIO_PIN               SYS_GPIO_PIN3
#define I2S_SDIN_GPIO_AF                5                                               /* AF功能选择 */
#define I2S_SDIN_GPIO_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 2; }while(0)           /* PC口时钟使能 */

#define I2S_MCLK_GPIO_PORT              GPIOC
#define I2S_MCLK_GPIO_PIN               SYS_GPIO_PIN6
#define I2S_MCLK_GPIO_AF                5                                               /* AF功能选择 */
#define I2S_MCLK_GPIO_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 2; }while(0)           /* PC口时钟使能 */


/* I2S相关定义 */
#define I2S_SPI                         SPI2
#define I2S_SPI_CLK_ENABLE()            do{ RCC->APB1ENR |= 1 << 14; }while(0)          /* I2S 时钟使能 */

/* I2S DMA相关定义 */
#define I2S_TX_DMASx                    DMA1_Stream4
#define I2S_TX_DMASx_Channel            0
#define I2S_TX_DMASx_IRQHandler         DMA1_Stream4_IRQHandler
#define I2S_TX_DMASx_IRQn               DMA1_Stream4_IRQn
#define I2S_TX_DMA_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 21; }while(0)          /* I2S TX DMA时钟使能 */

#define I2S_TX_DMASx_IS_TC()            ( DMA1->HISR & (1 << 5) )                       /* 判断 DMA1_Stream4 传输完成标志, 这是一个假函数形式,
                                                                                         * 不能当函数使用, 只能用在if等语句里面 
                                                                                         */
#define I2S_TX_DMASx_CLR_TC()           do{ DMA1->HIFCR |= 1 << 5; }while(0)            /* 清除 DMA1_Stream4 传输完成标志 */


/* I2Sext相关定义, 用于实现全双工(如录音机) */
#define I2SEXT_SPI                      I2S2ext


/* I2Sext RX DMA相关定义 */
#define I2SEXT_RX_DMASx                 DMA1_Stream3
#define I2SEXT_RX_DMASx_Channel         3
#define I2SEXT_RX_DMASx_IRQHandler      DMA1_Stream3_IRQHandler
#define I2SEXT_RX_DMASx_IRQn            DMA1_Stream3_IRQn
#define I2SEXT_RX_DMA_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 21; }while(0)          /* I2Sext TX DMA时钟使能 */

#define I2SEXT_RX_DMASx_IS_TC()         ( DMA1->LISR & (1 << 27) )                      /* 判断 DMA1_Stream3 传输完成标志, 这是一个假函数形式,
                                                                                         * 不能当函数使用, 只能用在if等语句里面 
                                                                                         */
#define I2SEXT_RX_DMASx_CLR_TC()        do{ DMA1->LIFCR |= 1 << 27; }while(0)           /* 清除 DMA1_Stream3 传输完成标志 */


/*****************************************************************************************************/

extern void (*i2s_tx_callback)(void);   /* I2S TX回调函数指针  */
extern void (*i2s_rx_callback)(void);   /* I2S RX回调函数指针  */

void i2s_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen);    /* I2S初始化 */
void i2sext_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen); /* I2S ext 初始化 */
uint8_t i2s_samplerate_set(uint32_t samplerate);                    /* 设置I2S的采样率 */
void i2s_tx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num);   /* I2S TX DMA配置 */
void i2sext_rx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num);/* I2S ext RX DMA配置 */
void i2s_play_start(void);              /* I2S开始播放 */
void i2s_play_stop(void);               /* I2S停止播放 */
void i2s_rec_start(void);               /* I2S开始录音 */
void i2s_rec_stop(void);                /* I2S停止录音 */

#endif





















