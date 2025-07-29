/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.3
 * @date        2022-01-09
 * @brief       ADC 驱动代码
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
 * V1.1 20220109
 * 1,支持ADC单通道DMA采集 
 * 2,新增adc_dma_init和adc_dma_enable函数
 * V1.2 20220109
 * 1,支持ADC多通道DMA采集 
 * 2,新增adc_nch_dma_init函数.
 * V1.3 20220109
 * 1,支持内部温度传感器温度采集
 * 2,新增adc_temperature_init和adc_get_temperature函数.
 *
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc.h"
#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"


uint8_t g_adc_dma_sta = 0;  /* DMA传输状态标志, 0,未完成; 1, 已完成 */

/**
 * @brief       ADC初始化函数
 *   @note      本函数支持ADC1/ADC2任意通道, 但是不支持ADC3
 *              我们使用12位精度, ADC采样时钟=21M, 转换时间为: 采样周期 + 12个ADC周期
 *              设置最大采样周期: 480, 则转换时间 = 492 个ADC周期 = 21.87us
 * @param       无
 * @retval      无
 */
void adc_init(void)
{
    ADC_ADCX_CHY_GPIO_CLK_ENABLE(); /* IO口时钟使能 */
    ADC_ADCX_CHY_CLK_ENABLE();      /* ADC时钟使能 */
    
    sys_gpio_set(ADC_ADCX_CHY_GPIO_PORT, ADC_ADCX_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */

    RCC->APB2RSTR |= 1 << 8;        /* ADC1 & ADC2 & ADC3 复位, 注意, 这里复位了所有ADC!!! */
    RCC->APB2RSTR &= ~(1 << 8);     /* 复位结束 */

    /* ADC时钟来自 APB2, 即PCLK2, 频率为84Mhz, ADC最大时钟一般不要超过36M
     * 在84M PCLK2条件下, 我们使用4分频, 得到PCLK2 / 4 = 21Mhz 的ADC时钟
     */
    ADC->CCR &= ~(3 << 16);         /* ADCPRE[1:0] ADC时钟预分频清零 */
    ADC->CCR |= 1 << 16;            /* 设置ADC时钟预分频系数为 4, 即 PCLK2 / 4 = 21Mhz */

    ADC_ADCX->CR1 = 0;              /* CR1清零 */
    ADC_ADCX->CR2 = 0;              /* CR2清零 */

    ADC_ADCX->CR1 |= 0 << 8;        /* 非扫描模式 */
    ADC_ADCX->CR1 |= 0 << 24;       /* 12位模式 */

    ADC_ADCX->CR2 |= 0 << 1;        /* 单次转换模式 */
    ADC_ADCX->CR2 |= 0 << 11;       /* 右对齐 */
    ADC_ADCX->CR2 |= 0 << 28;       /* 软件触发 */
    
    ADC_ADCX->SQR1 &= ~(0XF << 20); /* L[3:0]清零 */
    ADC_ADCX->SQR1 |= 0 << 20;      /* 1个转换在规则序列中 也就是只转换规则序列1 */

    ADC_ADCX->CR2 |= 1 << 0;        /* 开启AD转换器 */
}

/**
 * @brief       设置ADC通道采样时间
 * @param       adcx : adc结构体指针, ADC1 / ADC2
 * @param       ch   : 通道号, 0~18
 * @param       stime: 采样时间  0~7, 对应关系为:
 *   @arg       000, 3个ADC时钟周期          001, 15个ADC时钟周期
 *   @arg       010, 28个ADC时钟周期         011, 56个ADC时钟周期
 *   @arg       100, 84个ADC时钟周期         101, 112个ADC时钟周期
 *   @arg       110, 144个ADC时钟周期        111, 480个ADC时钟周期 
 * @retval      无
 */
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime)
{
    if (ch < 10)    /* 通道0~9,使用SMPR2配置 */
    { 
        adcx->SMPR2 &= ~(7 << (3 * ch));        /* 通道ch 采样时间清空 */
        adcx->SMPR2 |= 7 << (3 * ch);           /* 通道ch 采样周期设置,周期越高精度越高 */
    }
    else            /* 通道10~19,使用SMPR2配置 */
    { 
        adcx->SMPR1 &= ~(7 << (3 * (ch - 10))); /* 通道ch 采样时间清空 */
        adcx->SMPR1 |= 7 << (3 * (ch - 10));    /* 通道ch 采样周期设置,周期越高精度越高 */
    } 
}

/**
 * @brief       获得ADC转换后的结果 
 * @param       ch: 通道号, 0~18
 * @retval      无
 */
uint32_t adc_get_result(uint8_t ch)
{
    adc_channel_set(ADC_ADCX, ch, 7);   /* 设置ADCX对应通道采样时间为480个时钟周期 */

    ADC_ADCX->SQR3 &= ~(0X1F << 5 * 0); /* 规则序列1通道清零 */
    ADC_ADCX->SQR3 |= ch << (5 * 0);    /* 规则序列1 通道 = ch */
    ADC_ADCX->CR2 |= 1 << 30;           /* 启动规则转换通道 */

    while (!(ADC_ADCX->SR & 1 << 1));   /* 等待转换结束 */

    return ADC_ADCX->DR;                /* 返回adc值 */
}

/**
 * @brief       获取通道ch的转换值，取times次,然后平均
 * @param       ch      : 通道号, 0~19
 * @param       times   : 获取次数
 * @retval      通道ch的times次转换结果平均值
 */
uint32_t adc_get_result_average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++) /* 获取times次数据 */
    {
        temp_val += adc_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;    /* 返回平均值 */
}

/**
 * @brief       ADC DMA采集中断服务函数
 * @param       无 
 * @retval      无
 */
void ADC_ADCX_DMASx_IRQHandler(void)
{
    if (ADC_ADCX_DMASx_IS_TC())     /* DMA传输完成? */
    {
        g_adc_dma_sta = 1;          /* 标记DMA传输完成 */
        ADC_ADCX_DMASx_CLR_TC();    /* 清除DMA 数据流 传输完成中断标志 */
    }
}

/**
 * @brief       ADC DMA读取 初始化函数
 *   @note      本函数还是使用adc_init对ADC进行大部分配置,有差异的地方再单独配置
 * @param       mar         : 存储器地址 
 * @retval      无
 */
void adc_dma_init(uint32_t mar)
{
    adc_init(); /* 先初始化ADC */

    adc_channel_set(ADC_ADCX, ADC_ADCX_CHY, 7); /* 设置ADCX对应通道采样时间为480个时钟周期 */
    
    /* 设置转换序列 */
    /* 配置ADC连续转换, DMA传输ADC数据 */
    ADC_ADCX->CR2 |= 1 << 8;                    /* DMA = 1, DMA传输使能 */
    ADC_ADCX->CR2 |= 1 << 9;                    /* DDS = 1, 只要发生转换就请求DMA */
    ADC_ADCX->CR2 |= 1 << 1;                    /* CONT = 1, 连续转换模式 */

    ADC_ADCX->SQR3 &= ~(0X1F << 5 * 0);         /* 规则序列1通道清零 */
    ADC_ADCX->SQR3 |= ADC_ADCX_CHY << (5 * 0);  /* 规则序列1 通道 = ADC_ADCX_CHY */

    /* DMA基本设置, 数据流/通道/外设地址/存储器地址等 */
    dma_basic_config(ADC_ADCX_DMASx, ADC_ADCX_DMASx_Channel, (uint32_t)&ADC_ADCX->DR, mar, 0);

    /* 其他DMA配置 */
    ADC_ADCX_DMASx->CR |= 0 << 6;   /* 外设到存储器模式 */
    ADC_ADCX_DMASx->CR |= 0 << 8;   /* 非循环模式(即使用普通模式) */
    ADC_ADCX_DMASx->CR |= 0 << 9;   /* 外设非增量模式 */
    ADC_ADCX_DMASx->CR |= 1 << 10;  /* 存储器增量模式 */
    ADC_ADCX_DMASx->CR |= 1 << 11;  /* 外设数据长度:16位 */
    ADC_ADCX_DMASx->CR |= 1 << 13;  /* 存储器数据长度:16位 */
    ADC_ADCX_DMASx->CR |= 1 << 16;  /* 中等优先级 */
    ADC_ADCX_DMASx->CR |= 0 << 21;  /* 外设突发单次传输 */
    ADC_ADCX_DMASx->CR |= 0 << 23;  /* 存储器突发单次传输 */
    
    ADC_ADCX_DMASx->CR |= 1 << 4;   /* TCIE = 1, DMA传输完成中断使能 */

    sys_nvic_init(3, 3, ADC_ADCX_DMASx_IRQn, 2);    /* 组2，最低优先级 */
}

/**
 * @brief       使能一次ADC DMA传输 
 * @param       cndtr: DMA传输的次数
 * @retval      无
 */
void adc_dma_enable(uint16_t ndtr)
{
    ADC_ADCX->CR2 &= ~(1 << 0);         /* 先关闭ADC */
    
    dma_enable(ADC_ADCX_DMASx, ndtr);   /* 重新使能DMA传输 */
    
    ADC_ADCX->CR2 |= 1 << 0;            /* 重新启动ADC */
    ADC_ADCX->CR2 |= 1 << 30;           /* 启动规则转换通道 */
}

/**
 * @brief       ADC N通道(6通道) DMA读取 初始化函数
 *   @note      本函数还是使用adc_init对ADC进行大部分配置,有差异的地方再单独配置
 *              另外,由于本函数用到了6个通道, 宏定义会比较多内容, 因此,本函数就不采用宏定义的方式来修改通道了,
 *              直接在本函数里面修改, 这里我们默认使用PA0~PA5这6个通道.
 *
 *              注意: 本函数还是使用 ADC_ADCX(默认=ADC1) 和 ADC_ADCX_DMASx( DMA2_Stream4 ) 及其相关定义
 *              不要乱修改adc.h里面的这两部分内容, 必须在理解原理的基础上进行修改, 否则可能导致无法正常使用.
 *
 * @param       mar         : 存储器地址 
 * @retval      无
 */
void adc_nch_dma_init(uint32_t mar)
{
    adc_init(); /* 先初始化ADC */
    
    adc_channel_set(ADC_ADCX, 0, 7);/* 设置ADCX通道0采样时间为480个时钟周期 */
    adc_channel_set(ADC_ADCX, 1, 7);/* 设置ADCX通道1采样时间为480个时钟周期 */
    adc_channel_set(ADC_ADCX, 2, 7);/* 设置ADCX通道2采样时间为480个时钟周期 */
    adc_channel_set(ADC_ADCX, 3, 7);/* 设置ADCX通道3采样时间为480个时钟周期 */
    adc_channel_set(ADC_ADCX, 4, 7);/* 设置ADCX通道4采样时间为480个时钟周期 */
    adc_channel_set(ADC_ADCX, 5, 7);/* 设置ADCX通道5采样时间为480个时钟周期 */

    /* 设置转换序列 */
    /* 配置ADC连续转换, DMA传输ADC数据 */
    ADC_ADCX->CR1 |= 1 << 8;        /* SCAN = 1, 扫描模式 */
    ADC_ADCX->CR2 |= 1 << 8;        /* DMA = 1, DMA传输使能 */
    ADC_ADCX->CR2 |= 1 << 9;        /* DDS = 1, 只要发生转换就请求DMA */
    ADC_ADCX->CR2 |= 1 << 1;        /* CONT = 1, 连续转换模式 */

    ADC_ADCX->SQR1 &= ~(0XF << 20); /* L[3:0]清零 */
    ADC_ADCX->SQR1 |= 5 << 20;      /* 6个转换在规则序列中 也就是转换规则序列1~6 */
    
    ADC_ADCX->SQR3  = 0 << 0;       /* SQ1[4:0] = 0, 规则序列1 = 通道0(PA0) */
    ADC_ADCX->SQR3 |= 1 << 5;       /* SQ2[4:0] = 1, 规则序列2 = 通道1(PA1) */
    ADC_ADCX->SQR3 |= 2 << 10;      /* SQ3[4:0] = 2, 规则序列3 = 通道2(PA2) */
    ADC_ADCX->SQR3 |= 3 << 15;      /* SQ4[4:0] = 3, 规则序列4 = 通道3(PA3) */
    ADC_ADCX->SQR3 |= 4 << 20;      /* SQ5[4:0] = 4, 规则序列5 = 通道4(PA4) */
    ADC_ADCX->SQR3 |= 5 << 25;      /* SQ6[4:0] = 5, 规则序列6 = 通道5(PA5) */

    /* 设置ADC1通道0~5对应的IO口模拟输入 */
    /* PA0对应 ADC1_IN0 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN0,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */
    
    /* PA1对应 ADC1_IN1 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN1,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */
    
    /* PA2对应 ADC1_IN2 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN2,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */
    
    /* PA3对应 ADC1_IN3 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */
    
    /* PA4对应 ADC1_IN4 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN4,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */
                 
    /* PA5对应 ADC1_IN5*/
    sys_gpio_set(GPIOA, SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD采集引脚模式设置,模拟输入 */


    /* DMA基本设置, 数据流/通道/外设地址/存储器地址等 */
    dma_basic_config(ADC_ADCX_DMASx, ADC_ADCX_DMASx_Channel, (uint32_t)&ADC_ADCX->DR, mar, 0);

    /* 其他DMA配置 */
    ADC_ADCX_DMASx->CR |= 0 << 6;   /* 外设到存储器模式 */
    ADC_ADCX_DMASx->CR |= 0 << 8;   /* 非循环模式(即使用普通模式) */
    ADC_ADCX_DMASx->CR |= 0 << 9;   /* 外设非增量模式 */
    ADC_ADCX_DMASx->CR |= 1 << 10;  /* 存储器增量模式 */
    ADC_ADCX_DMASx->CR |= 1 << 11;  /* 外设数据长度:16位 */
    ADC_ADCX_DMASx->CR |= 1 << 13;  /* 存储器数据长度:16位 */
    ADC_ADCX_DMASx->CR |= 1 << 16;  /* 中等优先级 */
    ADC_ADCX_DMASx->CR |= 0 << 21;  /* 外设突发单次传输 */
    ADC_ADCX_DMASx->CR |= 0 << 23;  /* 存储器突发单次传输 */
    
    ADC_ADCX_DMASx->CR |= 1 << 4;   /* TCIE = 1, DMA传输完成中断使能 */

    sys_nvic_init(3, 3, ADC_ADCX_DMASx_IRQn, 2);    /* 组2，最低优先级 */
}

/**
 * @brief       ADC 内部温度传感器 初始化函数
 *   @note      本函数还是使用adc_init对ADC进行大部分配置,有差异的地方再单独配置
 *              注意: STM32F4内部温度传感器只连接在ADC1的通道16上, 其他ADC无法进行转换.
 *
 * @param       无
 * @retval      无
 */
void adc_temperature_init(void)
{
    adc_init();                                         /* 先初始化ADC */
    ADC->CCR |= 1 << 23;                                /* 使能内部温度传感器 */
}

/**
 * @brief       获取内部温度传感器温度值
 * @param       无
 * @retval      温度值(扩大了100倍,单位:℃.)
 */
short adc_get_temperature(void)
{
    uint32_t adcx;
    short result;
    double temperature;

    adcx = adc_get_result_average(ADC_TEMPSENSOR_CHX, 10);  /* 读取内部温度传感器通道,10次取平均 */
    temperature = (float)adcx*(3.3/4096);                   /* 获取电压值 */
    temperature = (temperature - 0.76)/0.0025 + 25;         /* 将电压值转换为温度值 */
    result = temperature *= 100;                            /* 扩大100倍. */
    return result;

}






































