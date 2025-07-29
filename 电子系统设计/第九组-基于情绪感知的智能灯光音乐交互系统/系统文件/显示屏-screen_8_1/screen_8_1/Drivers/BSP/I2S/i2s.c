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

#include "./BSP/I2S/i2s.h"  
#include "./SYSTEM/delay/delay.h"
#include "ucos_ii.h"


/**
 * @brief       I2S初始化
 * @param       std             : I2S标准,      0,飞利浦标准;      1,MSB对齐标准(右对齐);  2,LSB对齐标准(左对齐);  3,PCM标准
 * @param       mode            : I2S工作模式,  0,从机发送;        1,从机接收;             2,主机发送;            3,主机接收
 * @param       cpol            : I2S时钟极性   0,时钟低电平有效;   1,时钟高电平有效
 * @param       datalen         : 数据长度      0,16位标准;        1,16位扩展(frame=32bit);2,24位;                3,32位.
 * @retval      无
 */
void i2s_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    I2S_SPI_CLK_ENABLE();               /* I2S 时钟使能 */
    I2S_LRCK_GPIO_CLK_ENABLE();         /* I2S LRCK  GPIO 时钟使能 */
    I2S_SCLK_GPIO_CLK_ENABLE();         /* I2S SLCK  GPIO 时钟使能 */
    I2S_SDOUT_GPIO_CLK_ENABLE();        /* I2S SDOUT GPIO 时钟使能 */
    I2S_SDIN_GPIO_CLK_ENABLE();         /* I2S SDIN  GPIO 时钟使能 */
    I2S_MCLK_GPIO_CLK_ENABLE();         /* I2S MCLK  GPIO 时钟使能 */

    sys_gpio_set(I2S_LRCK_GPIO_PORT, I2S_LRCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LRCK引脚模式设置(复用输出) */

    sys_gpio_set(I2S_SCLK_GPIO_PORT, I2S_SCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SLCK引脚模式设置(复用输出) */

    sys_gpio_set(I2S_SDOUT_GPIO_PORT, I2S_SDOUT_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDOUT引脚模式设置(复用输出) */
    
    sys_gpio_set(I2S_SDIN_GPIO_PORT, I2S_SDIN_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDIN引脚模式设置(复用输出) */
    
    sys_gpio_set(I2S_MCLK_GPIO_PORT, I2S_MCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MCLK引脚模式设置(复用输出) */

    sys_gpio_af_set(I2S_LRCK_GPIO_PORT,  I2S_LRCK_GPIO_PIN,  I2S_LRCK_GPIO_AF);     /* LRCK脚, AF功能设置 */
    sys_gpio_af_set(I2S_SCLK_GPIO_PORT,  I2S_SCLK_GPIO_PIN,  I2S_SCLK_GPIO_AF);     /* SCLK脚, AF功能设置 */
    sys_gpio_af_set(I2S_SDOUT_GPIO_PORT, I2S_SDOUT_GPIO_PIN, I2S_SDOUT_GPIO_AF);    /* SDOUT脚, AF功能设置 */
    sys_gpio_af_set(I2S_SDIN_GPIO_PORT,  I2S_SDIN_GPIO_PIN,  I2S_SDIN_GPIO_AF);     /* SDIN脚, AF功能设置 */
    sys_gpio_af_set(I2S_MCLK_GPIO_PORT,  I2S_MCLK_GPIO_PIN,  I2S_MCLK_GPIO_AF);     /* MCLK脚, AF功能设置 */
    
    I2S_SPI->I2SCFGR = 0;               /* 全部设置为0 */
    I2S_SPI->I2SPR = 0X02;              /* 分频寄存器为默认值 */
    I2S_SPI->I2SCFGR |= 1 << 11;        /* 选择:I2S模式 */
    I2S_SPI->I2SCFGR |= mode << 8;      /* I2S工作模式设置 */
    I2S_SPI->I2SCFGR |= std << 4;       /* I2S标准设置 */
    I2S_SPI->I2SCFGR |= cpol << 3;      /* 空闲时钟电平设置 */

    if (datalen)                        /* 非标准16位长度 */
    {
        I2S_SPI->I2SCFGR |= 1 << 0;     /* Channel长度为32位 */
        datalen -= 1;
    }
    else 
    {
        I2S_SPI->I2SCFGR |= 0 << 0;    /* Channel长度为16位 */
    }
    
    I2S_SPI->I2SCFGR |= datalen << 1;  /* I2S标准设置 */
    I2S_SPI->CR2 |= 1 << 1;            /* I2S_SPI TX DMA请求使能 */
    I2S_SPI->I2SCFGR |= 1 << 10;       /* I2S_SPI I2S EN使能 */
}

/**
 * @brief       I2S ext配置,用于全双工模式配置, 需要时, 请在i2s_init之后调用
 * @param       std             : I2S标准,      0,飞利浦标准;      1,MSB对齐标准(右对齐);  2,LSB对齐标准(左对齐);  3,PCM标准
 * @param       mode            : I2S工作模式,  0,从机发送;        1,从机接收;             2,主机发送;            3,主机接收
 * @param       cpol            : I2S时钟极性   0,时钟低电平有效;   1,时钟高电平有效
 * @param       datalen         : 数据长度      0,16位标准;        1,16位扩展(frame=32bit);2,24位;                3,32位.
 * @retval      无
 */
void i2sext_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    I2SEXT_SPI->I2SCFGR = 0;            /* 全部设置为0 */
    I2SEXT_SPI->I2SPR = 0X02;           /* 分频寄存器为默认值 */
    I2SEXT_SPI->I2SCFGR |= 1 << 11;     /* 选择:I2S模式 */
    I2SEXT_SPI->I2SCFGR |= mode << 8;   /* I2S工作模式设置 */
    I2SEXT_SPI->I2SCFGR |= std << 4;    /* I2S标准设置 */
    I2SEXT_SPI->I2SCFGR |= cpol << 3;   /* 空闲时钟电平设置 */

    if (datalen)                        /* 非标准16位长度 */
    {
        I2SEXT_SPI->I2SCFGR |= 1 << 0;  /* Channel长度为32位 */
        datalen -= 1;
    }
    else I2SEXT_SPI->I2SCFGR |= 0 << 0; /* Channel长度为16位 */

    I2SEXT_SPI->I2SCFGR |= datalen << 1;/* I2S标准设置 */
    I2SEXT_SPI->CR2 |= 1 << 0;          /* I2SEXT_SPI RX DMA请求使能 */
    I2SEXT_SPI->I2SCFGR |= 1 << 10;     /* I2SEXT_SPI I2S EN使能 */
}

/**
 * 采样率计算公式:Fs = I2SxCLK / [256 * (2 * I2SDIV + ODD)]
 * I2SxCLK = (HSE / pllm) * PLLI2SN / PLLI2SR
 * 一般HSE = 8Mhz 
 * pllm:在 sys_stm32_clock_init 设置的时候确定，一般是8
 * PLLI2SN:一般是192~432
 * PLLI2SR:2~7
 * I2SDIV:2~255
 * ODD:0/1
 * I2S分频系数表@pllm = 8,HSE = 8Mhz, 即vco输入频率为1Mhz
 * 表格式:采样率 / 10,PLLI2SN,PLLI2SR,I2SDIV,ODD
 */
const uint16_t I2S_PSC_TBL[][5]=
{
    {   800, 256, 5, 12, 1 },   /* 8Khz采样率 */
    {  1102, 429, 4, 19, 0 },   /* 11.025Khz采样率 */
    {  1600, 213, 2, 13, 0 },   /* 16Khz采样率 */
    {  2205, 429, 4,  9, 1 },   /* 22.05Khz采样率 */
    {  3200, 213, 2,  6, 1 },   /* 32Khz采样率 */
    {  4410, 271, 2,  6, 0 },   /* 44.1Khz采样率 */
    {  4800, 258, 3,  3, 1 },   /* 48Khz采样率 */
    {  8820, 316, 2,  3, 1 },   /* 88.2Khz采样率 */
    {  9600, 344, 2,  3, 1 },   /* 96Khz采样率 */
    { 17640, 361, 2,  2, 0 },   /* 176.4Khz采样率 */
    { 19200, 393, 2,  2, 0 },   /* 192Khz采样率 */
};

/**
 * @brief       设置I2S的采样率
 * @param       samplerate:采样率, 单位:Hz
 * @retval      0,设置成功
 *              1,无法设置
 */
uint8_t i2s_samplerate_set(uint32_t samplerate)
{   
    uint8_t i = 0; 
    uint32_t tempreg = 0;
    samplerate /= 10;                       /* 缩小10倍 */

    for (i = 0; i < (sizeof(I2S_PSC_TBL) / 10); i++) /* 看看改采样率是否可以支持 */
    {
        if (samplerate == I2S_PSC_TBL[i][0])
        {
            break;
        }
    }

    RCC->CR &= ~(1 << 26);                  /* 先关闭PLLI2S */

    if (i == (sizeof(I2S_PSC_TBL) / 10))    /* 搜遍了也找不到 */
    {
        return 1; 
    }
    
    tempreg |= (uint32_t)I2S_PSC_TBL[i][1] << 6;    /* 设置PLLI2SN */
    tempreg |= (uint32_t)I2S_PSC_TBL[i][2] << 28;   /* 设置PLLI2SR */
    RCC->PLLI2SCFGR = tempreg;                      /* 设置I2SxCLK的频率(x=2) */
    RCC->CR |= 1 << 26;                             /* 开启PLLI2S */

    while ((RCC->CR & 1 << 27) == 0);               /* 等待PLLI2S开启成功 */

    tempreg = I2S_PSC_TBL[i][3] << 0;               /* 设置I2SDIV */
    tempreg |= I2S_PSC_TBL[i][4] << 8;              /* 设置ODD位 */
    tempreg |= 1 << 9;                              /* 使能MCKOE位,输出MCK */
    I2S_SPI->I2SPR = tempreg;                       /* 设置I2SPR寄存器 */

    return 0;
}

/**
 * @brief       I2S TX DMA配置
 *  @note       设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0 : M0AR地址.
 * @param       buf1 : M1AR地址.
 * @param       num  : 每次传输数据量
 * @retval      无
 */
void i2s_tx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num)
{  
    I2S_TX_DMA_CLK_ENABLE();            /* I2S TX DMA时钟使能 */

    I2S_TX_DMASx_CLR_TC();              /* 清除一次中断 */
    while (I2S_TX_DMASx->CR & 0X01);    /* 等待I2S_TX_DMASx可配置 */

    I2S_TX_DMASx->FCR = 0X0000021;      /* 设置为默认值 */

    I2S_TX_DMASx->PAR = (uint32_t)&I2S_SPI->DR; /* 外设地址为:I2S_SPI->DR */
    I2S_TX_DMASx->M0AR = (uint32_t)buf0;/* 内存1地址 */
    I2S_TX_DMASx->M1AR = (uint32_t)buf1;/* 内存2地址 */
    I2S_TX_DMASx->NDTR = num;           /* 暂时设置长度为1 */
    I2S_TX_DMASx->CR = 0;               /* 先全部复位CR寄存器值 */
    I2S_TX_DMASx->CR |= 1 << 6;         /* 存储器到外设模式 */
    I2S_TX_DMASx->CR |= 1 << 8;         /* 循环模式 */
    I2S_TX_DMASx->CR |= 0 << 9;         /* 外设非增量模式 */
    I2S_TX_DMASx->CR |= 1 << 10;        /* 存储器增量模式 */
    I2S_TX_DMASx->CR |= 1 << 11;        /* 外设数据长度:16位 */
    I2S_TX_DMASx->CR |= 1 << 13;        /* 存储器数据长度:16位 */
    I2S_TX_DMASx->CR |= 2 << 16;        /* 高优先级 */
    I2S_TX_DMASx->CR |= 1 << 18;        /* 双缓冲模式 */
    I2S_TX_DMASx->CR |= 0 << 21;        /* 外设突发单次传输 */
    I2S_TX_DMASx->CR |= 0 << 23;        /* 存储器突发单次传输 */
    I2S_TX_DMASx->CR |= I2S_TX_DMASx_Channel << 25; /* 选择通道 */

    I2S_TX_DMASx->FCR &= ~(1 << 2);     /* 不使用FIFO模式 */
    I2S_TX_DMASx->FCR &= ~(3 << 0);     /* 无FIFO 设置 */

    I2S_TX_DMASx->CR |= 1 << 4;         /* 开启传输完成中断 */

    sys_nvic_init(0, 0, I2S_TX_DMASx_IRQn, 2);  /* 抢占1，子优先级0，组2 */
} 

/**
 * @brief       I2Sext RX DMA配置
 *  @note       设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0 : M0AR地址.
 * @param       buf1 : M1AR地址.
 * @param       num  : 每次传输数据量
 * @retval      无
 */
void i2sext_rx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num)
{  
    I2SEXT_RX_DMA_CLK_ENABLE();         /* I2Sext RX DMA时钟使能 */

    while (I2SEXT_RX_DMASx->CR & 0X01); /* 等待I2SEXT_RX_DMASx可配置 */

    I2SEXT_RX_DMASx_CLR_TC();           /* 清除一次中断 */
    I2SEXT_RX_DMASx->FCR = 0X0000021;   /* 设置为默认值 */

    I2SEXT_RX_DMASx->PAR = (uint32_t)&I2SEXT_SPI->DR;   /* 外设地址为:I2SEXT_SPI->DR */
    I2SEXT_RX_DMASx->M0AR = (uint32_t)buf0;             /* 内存1地址 */
    I2SEXT_RX_DMASx->M1AR = (uint32_t)buf1;             /* 内存2地址 */
    I2SEXT_RX_DMASx->NDTR = num;        /* 暂时设置长度为1 */
    I2SEXT_RX_DMASx->CR = 0;            /* 先全部复位CR寄存器值 */
    I2SEXT_RX_DMASx->CR |= 0 << 6;      /* 外设到存储器模式 */
    I2SEXT_RX_DMASx->CR |= 1 << 8;      /* 循环模式 */
    I2SEXT_RX_DMASx->CR |= 0 << 9;      /* 外设非增量模式 */
    I2SEXT_RX_DMASx->CR |= 1 << 10;     /* 存储器增量模式 */
    I2SEXT_RX_DMASx->CR |= 1 << 11;     /* 外设数据长度:16位 */
    I2SEXT_RX_DMASx->CR |= 1 << 13;     /* 存储器数据长度:16位 */
    I2SEXT_RX_DMASx->CR |= 1 << 16;     /* 中等优先级 */
    I2SEXT_RX_DMASx->CR |= 1 << 18;     /* 双缓冲模式 */
    I2SEXT_RX_DMASx->CR |= 0 << 21;     /* 外设突发单次传输 */
    I2SEXT_RX_DMASx->CR |= 0 << 23;     /* 存储器突发单次传输 */
    I2SEXT_RX_DMASx->CR |= I2SEXT_RX_DMASx_Channel << 25;   /* 选择通道 */

    I2SEXT_RX_DMASx->FCR &= ~(1 << 2);  /* 不使用FIFO模式 */
    I2SEXT_RX_DMASx->FCR &= ~(3 << 0);  /* 无FIFO 设置 */

    I2SEXT_RX_DMASx->CR |= 1 << 4;      /* 开启传输完成中断 */

    sys_nvic_init(0, 1, I2SEXT_RX_DMASx_IRQn, 2);   /* 抢占1，子优先级0，组2 */
} 


void (*i2s_tx_callback)(void);          /* I2S TX DMA回调函数指针 */
void (*i2s_rx_callback)(void);          /* I2S RX DMA回调函数指针 */
 
/**
 * @brief       I2S TX DMA 中断服务函数
 * @param       无
 * @retval      无
 */
void I2S_TX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    
    if (I2S_TX_DMASx_IS_TC())           /* I2S TX DMA 传输完成? */
    {
        I2S_TX_DMASx_CLR_TC();          /* 清除 I2S TX DMA 传输完成中断标志 */
        i2s_tx_callback();              /* 执行回调函数,读取数据等操作在这里面处理 */
    }
    
    OSIntExit();
}

/**
 * @brief       I2Sext RX DMA 中断服务函数
 * @param       无
 * @retval      无
 */
void I2SEXT_RX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    
    if (I2SEXT_RX_DMASx_IS_TC())        /* I2Sext RX DMA 传输完成? */
    {
        I2SEXT_RX_DMASx_CLR_TC();       /* 清除 I2Sext RX DMA 传输完成中断标志 */
        i2s_rx_callback();              /* 执行回调函数,读取数据等操作在这里面处理 */
    }
    
    OSIntExit();
}

/**
 * @brief       I2S开始播放
 * @param       无
 * @retval      无
 */
void i2s_play_start(void)
{
    I2S_TX_DMASx->CR |= 1 << 0;;        /* 开启I2S TX DMA传输 */
}

/**
 * @brief       I2S停止播放
 * @param       无
 * @retval      无
 */
void i2s_play_stop(void)
{
    I2S_TX_DMASx->CR &= ~(1 << 0);      /* 关闭I2S TX DMA传输 */
}

/**
 * @brief       I2S开始录音
 * @param       无
 * @retval      无
 */
void i2s_rec_start(void)
{
    I2SEXT_RX_DMASx->CR |= 1 << 0;;     /* 开启I2S RX DMA传输 */
}

/**
 * @brief       I2S停止录音
 * @param       无
 * @retval      无
 */
void i2s_rec_stop(void)
{
    I2SEXT_RX_DMASx->CR &= ~(1 << 0);   /* 关闭I2S RX DMA传输 */
}

























