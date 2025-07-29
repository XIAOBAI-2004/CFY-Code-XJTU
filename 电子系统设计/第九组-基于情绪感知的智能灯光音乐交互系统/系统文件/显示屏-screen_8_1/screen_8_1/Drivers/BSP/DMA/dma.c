/**
 ****************************************************************************************************
 * @file        dma.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-09
 * @brief       DMA 驱动代码
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
 * V1.0 20220109
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       串口TX DMA初始化函数
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       dma_streamx : DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch         :  DMA通道选择,范围:1~115(详见<<STM32H7xx参考手册>>16.3.2节,Table 116)
 * @param       par         : 外设地址
 * @param       mar         : 存储器地址
 * @retval      无
 */
void dma_usart_tx_config(DMA_Stream_TypeDef *dma_streamx, uint8_t ch, uint32_t par, uint32_t mar)
{
    /* DMA基本配置 包括数据流/通道/外设地址/存储器地址等 */
    dma_basic_config(dma_streamx, ch, par, mar, 0);

    /* DMA特定参数具体配置 */
    dma_streamx->CR |= 1 << 6;      /* 存储器到外设模式 */
    dma_streamx->CR |= 0 << 8;      /* 非循环模式(即使用普通模式) */
    dma_streamx->CR |= 0 << 9;      /* 外设非增量模式 */
    dma_streamx->CR |= 1 << 10;     /* 存储器增量模式 */
    dma_streamx->CR |= 0 << 11;     /* 外设数据长度:8位 */
    dma_streamx->CR |= 0 << 13;     /* 存储器数据长度:8位 */
    dma_streamx->CR |= 1 << 16;     /* 中等优先级 */
    dma_streamx->CR |= 0 << 21;     /* 外设突发单次传输 */
    dma_streamx->CR |= 0 << 23;     /* 存储器突发单次传输 */
    
    //dma_streamx->FCR = 0X21;      /* FIFO控制寄存器 */
}

/**
 * @brief       DMA基本配置
 *   @note      这里仅对DMA完成一些基础性的配置, 包括: DMA时钟使能 / 设置外设地址 和 存储器地址
 *              其他配置参数(CCR寄存器), 需用户自己另外实现
 *
 * @param       dma_streamx : DMA及数据流, DMA1_Stream0 ~ DMA1_Stream7, DMA2_Stream0 ~ DMA1_Stream7
 * @param       ch          : 通道x, 0~7
 *                            具体某个外设对应哪个DMA, 哪个数据流, 哪个通道, 请参考<<STM32F4xx中文参考手册>> 9.3.3节
 *                            必须设置正确的DMA及通道, 才能正常使用! 
 * @param       par         : 外设地址
 * @param       m0ar        : 存储器0地址
 * @param       m1ar        : 存储器1地址, 使用双缓存的时候才会用到
 * @retval      无
 */
void dma_basic_config(DMA_Stream_TypeDef *dma_streamx,uint32_t ch, uint32_t par, uint32_t m0ar, uint32_t m1ar)
{
    if (dma_streamx > DMA1_Stream7)     /* 大于 DMA1_Stream7, 则为DMA2的通道了 */
    {
        RCC->AHB1ENR |= 1 << 22;        /* 开启DMA2时钟 */
    }
    else
    {
        RCC->AHB1ENR |= 1 << 21;        /* 开启DMA1时钟 */
    }

    delay_ms(5);                        /* 等待DMA时钟稳定 */

    dma_streamx->CR = (ch & 7) << 25;   /* 数据流通道选择, 0 ~ 7 */
    dma_streamx->PAR = par;             /* DMA 外设地址 */
    dma_streamx->M0AR = m0ar;           /* DMA 存储器0地址 */
    dma_streamx->M1AR = m1ar;           /* DMA 存储器1地址 */
    dma_streamx->NDTR = 0;              /* DMA 传输长度清零, 后续在dma_enable函数设置 */
}

/**
 * @brief       开启一次DMA传输
 * @param       dma_streamx : DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ndtr        : 数据传输量
 * @retval      无
 */
void dma_enable(DMA_Stream_TypeDef *dma_streamx, uint16_t ndtr)
{
    dma_streamx->CR &= ~(1 << 0);   /* 关闭DMA传输 */

    while (dma_streamx->CR & 0X1);  /* 确保DMA可以被设置 */

    dma_streamx->NDTR = ndtr;       /* 要传输的数据项数目 */
    dma_streamx->CR |= 1 << 0;      /* 开启DMA传输 */
}



























