/**
 ****************************************************************************************************
 * @file        i2s.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-01-16
 * @brief       I2S ��������
 *              ����ʹ�õ���I2S2���I2S
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
 * V1.0 20220114
 * ��һ�η���
 * V1.1 20220116
 * ���� i2sext_init, i2sext_rx_dma_init, i2s_rec_start��i2s_rec_stop��4������
 *
 ****************************************************************************************************
 */

#include "./BSP/I2S/i2s.h"  
#include "./SYSTEM/delay/delay.h"
#include "ucos_ii.h"


/**
 * @brief       I2S��ʼ��
 * @param       std             : I2S��׼,      0,�����ֱ�׼;      1,MSB�����׼(�Ҷ���);  2,LSB�����׼(�����);  3,PCM��׼
 * @param       mode            : I2S����ģʽ,  0,�ӻ�����;        1,�ӻ�����;             2,��������;            3,��������
 * @param       cpol            : I2Sʱ�Ӽ���   0,ʱ�ӵ͵�ƽ��Ч;   1,ʱ�Ӹߵ�ƽ��Ч
 * @param       datalen         : ���ݳ���      0,16λ��׼;        1,16λ��չ(frame=32bit);2,24λ;                3,32λ.
 * @retval      ��
 */
void i2s_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    I2S_SPI_CLK_ENABLE();               /* I2S ʱ��ʹ�� */
    I2S_LRCK_GPIO_CLK_ENABLE();         /* I2S LRCK  GPIO ʱ��ʹ�� */
    I2S_SCLK_GPIO_CLK_ENABLE();         /* I2S SLCK  GPIO ʱ��ʹ�� */
    I2S_SDOUT_GPIO_CLK_ENABLE();        /* I2S SDOUT GPIO ʱ��ʹ�� */
    I2S_SDIN_GPIO_CLK_ENABLE();         /* I2S SDIN  GPIO ʱ��ʹ�� */
    I2S_MCLK_GPIO_CLK_ENABLE();         /* I2S MCLK  GPIO ʱ��ʹ�� */

    sys_gpio_set(I2S_LRCK_GPIO_PORT, I2S_LRCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LRCK����ģʽ����(�������) */

    sys_gpio_set(I2S_SCLK_GPIO_PORT, I2S_SCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SLCK����ģʽ����(�������) */

    sys_gpio_set(I2S_SDOUT_GPIO_PORT, I2S_SDOUT_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDOUT����ģʽ����(�������) */
    
    sys_gpio_set(I2S_SDIN_GPIO_PORT, I2S_SDIN_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SDIN����ģʽ����(�������) */
    
    sys_gpio_set(I2S_MCLK_GPIO_PORT, I2S_MCLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MCLK����ģʽ����(�������) */

    sys_gpio_af_set(I2S_LRCK_GPIO_PORT,  I2S_LRCK_GPIO_PIN,  I2S_LRCK_GPIO_AF);     /* LRCK��, AF�������� */
    sys_gpio_af_set(I2S_SCLK_GPIO_PORT,  I2S_SCLK_GPIO_PIN,  I2S_SCLK_GPIO_AF);     /* SCLK��, AF�������� */
    sys_gpio_af_set(I2S_SDOUT_GPIO_PORT, I2S_SDOUT_GPIO_PIN, I2S_SDOUT_GPIO_AF);    /* SDOUT��, AF�������� */
    sys_gpio_af_set(I2S_SDIN_GPIO_PORT,  I2S_SDIN_GPIO_PIN,  I2S_SDIN_GPIO_AF);     /* SDIN��, AF�������� */
    sys_gpio_af_set(I2S_MCLK_GPIO_PORT,  I2S_MCLK_GPIO_PIN,  I2S_MCLK_GPIO_AF);     /* MCLK��, AF�������� */
    
    I2S_SPI->I2SCFGR = 0;               /* ȫ������Ϊ0 */
    I2S_SPI->I2SPR = 0X02;              /* ��Ƶ�Ĵ���ΪĬ��ֵ */
    I2S_SPI->I2SCFGR |= 1 << 11;        /* ѡ��:I2Sģʽ */
    I2S_SPI->I2SCFGR |= mode << 8;      /* I2S����ģʽ���� */
    I2S_SPI->I2SCFGR |= std << 4;       /* I2S��׼���� */
    I2S_SPI->I2SCFGR |= cpol << 3;      /* ����ʱ�ӵ�ƽ���� */

    if (datalen)                        /* �Ǳ�׼16λ���� */
    {
        I2S_SPI->I2SCFGR |= 1 << 0;     /* Channel����Ϊ32λ */
        datalen -= 1;
    }
    else 
    {
        I2S_SPI->I2SCFGR |= 0 << 0;    /* Channel����Ϊ16λ */
    }
    
    I2S_SPI->I2SCFGR |= datalen << 1;  /* I2S��׼���� */
    I2S_SPI->CR2 |= 1 << 1;            /* I2S_SPI TX DMA����ʹ�� */
    I2S_SPI->I2SCFGR |= 1 << 10;       /* I2S_SPI I2S ENʹ�� */
}

/**
 * @brief       I2S ext����,����ȫ˫��ģʽ����, ��Ҫʱ, ����i2s_init֮�����
 * @param       std             : I2S��׼,      0,�����ֱ�׼;      1,MSB�����׼(�Ҷ���);  2,LSB�����׼(�����);  3,PCM��׼
 * @param       mode            : I2S����ģʽ,  0,�ӻ�����;        1,�ӻ�����;             2,��������;            3,��������
 * @param       cpol            : I2Sʱ�Ӽ���   0,ʱ�ӵ͵�ƽ��Ч;   1,ʱ�Ӹߵ�ƽ��Ч
 * @param       datalen         : ���ݳ���      0,16λ��׼;        1,16λ��չ(frame=32bit);2,24λ;                3,32λ.
 * @retval      ��
 */
void i2sext_init(uint32_t std, uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    I2SEXT_SPI->I2SCFGR = 0;            /* ȫ������Ϊ0 */
    I2SEXT_SPI->I2SPR = 0X02;           /* ��Ƶ�Ĵ���ΪĬ��ֵ */
    I2SEXT_SPI->I2SCFGR |= 1 << 11;     /* ѡ��:I2Sģʽ */
    I2SEXT_SPI->I2SCFGR |= mode << 8;   /* I2S����ģʽ���� */
    I2SEXT_SPI->I2SCFGR |= std << 4;    /* I2S��׼���� */
    I2SEXT_SPI->I2SCFGR |= cpol << 3;   /* ����ʱ�ӵ�ƽ���� */

    if (datalen)                        /* �Ǳ�׼16λ���� */
    {
        I2SEXT_SPI->I2SCFGR |= 1 << 0;  /* Channel����Ϊ32λ */
        datalen -= 1;
    }
    else I2SEXT_SPI->I2SCFGR |= 0 << 0; /* Channel����Ϊ16λ */

    I2SEXT_SPI->I2SCFGR |= datalen << 1;/* I2S��׼���� */
    I2SEXT_SPI->CR2 |= 1 << 0;          /* I2SEXT_SPI RX DMA����ʹ�� */
    I2SEXT_SPI->I2SCFGR |= 1 << 10;     /* I2SEXT_SPI I2S ENʹ�� */
}

/**
 * �����ʼ��㹫ʽ:Fs = I2SxCLK / [256 * (2 * I2SDIV + ODD)]
 * I2SxCLK = (HSE / pllm) * PLLI2SN / PLLI2SR
 * һ��HSE = 8Mhz 
 * pllm:�� sys_stm32_clock_init ���õ�ʱ��ȷ����һ����8
 * PLLI2SN:һ����192~432
 * PLLI2SR:2~7
 * I2SDIV:2~255
 * ODD:0/1
 * I2S��Ƶϵ����@pllm = 8,HSE = 8Mhz, ��vco����Ƶ��Ϊ1Mhz
 * ���ʽ:������ / 10,PLLI2SN,PLLI2SR,I2SDIV,ODD
 */
const uint16_t I2S_PSC_TBL[][5]=
{
    {   800, 256, 5, 12, 1 },   /* 8Khz������ */
    {  1102, 429, 4, 19, 0 },   /* 11.025Khz������ */
    {  1600, 213, 2, 13, 0 },   /* 16Khz������ */
    {  2205, 429, 4,  9, 1 },   /* 22.05Khz������ */
    {  3200, 213, 2,  6, 1 },   /* 32Khz������ */
    {  4410, 271, 2,  6, 0 },   /* 44.1Khz������ */
    {  4800, 258, 3,  3, 1 },   /* 48Khz������ */
    {  8820, 316, 2,  3, 1 },   /* 88.2Khz������ */
    {  9600, 344, 2,  3, 1 },   /* 96Khz������ */
    { 17640, 361, 2,  2, 0 },   /* 176.4Khz������ */
    { 19200, 393, 2,  2, 0 },   /* 192Khz������ */
};

/**
 * @brief       ����I2S�Ĳ�����
 * @param       samplerate:������, ��λ:Hz
 * @retval      0,���óɹ�
 *              1,�޷�����
 */
uint8_t i2s_samplerate_set(uint32_t samplerate)
{   
    uint8_t i = 0; 
    uint32_t tempreg = 0;
    samplerate /= 10;                       /* ��С10�� */

    for (i = 0; i < (sizeof(I2S_PSC_TBL) / 10); i++) /* �����Ĳ������Ƿ����֧�� */
    {
        if (samplerate == I2S_PSC_TBL[i][0])
        {
            break;
        }
    }

    RCC->CR &= ~(1 << 26);                  /* �ȹر�PLLI2S */

    if (i == (sizeof(I2S_PSC_TBL) / 10))    /* �ѱ���Ҳ�Ҳ��� */
    {
        return 1; 
    }
    
    tempreg |= (uint32_t)I2S_PSC_TBL[i][1] << 6;    /* ����PLLI2SN */
    tempreg |= (uint32_t)I2S_PSC_TBL[i][2] << 28;   /* ����PLLI2SR */
    RCC->PLLI2SCFGR = tempreg;                      /* ����I2SxCLK��Ƶ��(x=2) */
    RCC->CR |= 1 << 26;                             /* ����PLLI2S */

    while ((RCC->CR & 1 << 27) == 0);               /* �ȴ�PLLI2S�����ɹ� */

    tempreg = I2S_PSC_TBL[i][3] << 0;               /* ����I2SDIV */
    tempreg |= I2S_PSC_TBL[i][4] << 8;              /* ����ODDλ */
    tempreg |= 1 << 9;                              /* ʹ��MCKOEλ,���MCK */
    I2S_SPI->I2SPR = tempreg;                       /* ����I2SPR�Ĵ��� */

    return 0;
}

/**
 * @brief       I2S TX DMA����
 *  @note       ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0 : M0AR��ַ.
 * @param       buf1 : M1AR��ַ.
 * @param       num  : ÿ�δ���������
 * @retval      ��
 */
void i2s_tx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num)
{  
    I2S_TX_DMA_CLK_ENABLE();            /* I2S TX DMAʱ��ʹ�� */

    I2S_TX_DMASx_CLR_TC();              /* ���һ���ж� */
    while (I2S_TX_DMASx->CR & 0X01);    /* �ȴ�I2S_TX_DMASx������ */

    I2S_TX_DMASx->FCR = 0X0000021;      /* ����ΪĬ��ֵ */

    I2S_TX_DMASx->PAR = (uint32_t)&I2S_SPI->DR; /* �����ַΪ:I2S_SPI->DR */
    I2S_TX_DMASx->M0AR = (uint32_t)buf0;/* �ڴ�1��ַ */
    I2S_TX_DMASx->M1AR = (uint32_t)buf1;/* �ڴ�2��ַ */
    I2S_TX_DMASx->NDTR = num;           /* ��ʱ���ó���Ϊ1 */
    I2S_TX_DMASx->CR = 0;               /* ��ȫ����λCR�Ĵ���ֵ */
    I2S_TX_DMASx->CR |= 1 << 6;         /* �洢��������ģʽ */
    I2S_TX_DMASx->CR |= 1 << 8;         /* ѭ��ģʽ */
    I2S_TX_DMASx->CR |= 0 << 9;         /* ���������ģʽ */
    I2S_TX_DMASx->CR |= 1 << 10;        /* �洢������ģʽ */
    I2S_TX_DMASx->CR |= 1 << 11;        /* �������ݳ���:16λ */
    I2S_TX_DMASx->CR |= 1 << 13;        /* �洢�����ݳ���:16λ */
    I2S_TX_DMASx->CR |= 2 << 16;        /* �����ȼ� */
    I2S_TX_DMASx->CR |= 1 << 18;        /* ˫����ģʽ */
    I2S_TX_DMASx->CR |= 0 << 21;        /* ����ͻ�����δ��� */
    I2S_TX_DMASx->CR |= 0 << 23;        /* �洢��ͻ�����δ��� */
    I2S_TX_DMASx->CR |= I2S_TX_DMASx_Channel << 25; /* ѡ��ͨ�� */

    I2S_TX_DMASx->FCR &= ~(1 << 2);     /* ��ʹ��FIFOģʽ */
    I2S_TX_DMASx->FCR &= ~(3 << 0);     /* ��FIFO ���� */

    I2S_TX_DMASx->CR |= 1 << 4;         /* ������������ж� */

    sys_nvic_init(0, 0, I2S_TX_DMASx_IRQn, 2);  /* ��ռ1�������ȼ�0����2 */
} 

/**
 * @brief       I2Sext RX DMA����
 *  @note       ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0 : M0AR��ַ.
 * @param       buf1 : M1AR��ַ.
 * @param       num  : ÿ�δ���������
 * @retval      ��
 */
void i2sext_rx_dma_init(uint8_t* buf0, uint8_t *buf1, uint16_t num)
{  
    I2SEXT_RX_DMA_CLK_ENABLE();         /* I2Sext RX DMAʱ��ʹ�� */

    while (I2SEXT_RX_DMASx->CR & 0X01); /* �ȴ�I2SEXT_RX_DMASx������ */

    I2SEXT_RX_DMASx_CLR_TC();           /* ���һ���ж� */
    I2SEXT_RX_DMASx->FCR = 0X0000021;   /* ����ΪĬ��ֵ */

    I2SEXT_RX_DMASx->PAR = (uint32_t)&I2SEXT_SPI->DR;   /* �����ַΪ:I2SEXT_SPI->DR */
    I2SEXT_RX_DMASx->M0AR = (uint32_t)buf0;             /* �ڴ�1��ַ */
    I2SEXT_RX_DMASx->M1AR = (uint32_t)buf1;             /* �ڴ�2��ַ */
    I2SEXT_RX_DMASx->NDTR = num;        /* ��ʱ���ó���Ϊ1 */
    I2SEXT_RX_DMASx->CR = 0;            /* ��ȫ����λCR�Ĵ���ֵ */
    I2SEXT_RX_DMASx->CR |= 0 << 6;      /* ���赽�洢��ģʽ */
    I2SEXT_RX_DMASx->CR |= 1 << 8;      /* ѭ��ģʽ */
    I2SEXT_RX_DMASx->CR |= 0 << 9;      /* ���������ģʽ */
    I2SEXT_RX_DMASx->CR |= 1 << 10;     /* �洢������ģʽ */
    I2SEXT_RX_DMASx->CR |= 1 << 11;     /* �������ݳ���:16λ */
    I2SEXT_RX_DMASx->CR |= 1 << 13;     /* �洢�����ݳ���:16λ */
    I2SEXT_RX_DMASx->CR |= 1 << 16;     /* �е����ȼ� */
    I2SEXT_RX_DMASx->CR |= 1 << 18;     /* ˫����ģʽ */
    I2SEXT_RX_DMASx->CR |= 0 << 21;     /* ����ͻ�����δ��� */
    I2SEXT_RX_DMASx->CR |= 0 << 23;     /* �洢��ͻ�����δ��� */
    I2SEXT_RX_DMASx->CR |= I2SEXT_RX_DMASx_Channel << 25;   /* ѡ��ͨ�� */

    I2SEXT_RX_DMASx->FCR &= ~(1 << 2);  /* ��ʹ��FIFOģʽ */
    I2SEXT_RX_DMASx->FCR &= ~(3 << 0);  /* ��FIFO ���� */

    I2SEXT_RX_DMASx->CR |= 1 << 4;      /* ������������ж� */

    sys_nvic_init(0, 1, I2SEXT_RX_DMASx_IRQn, 2);   /* ��ռ1�������ȼ�0����2 */
} 


void (*i2s_tx_callback)(void);          /* I2S TX DMA�ص�����ָ�� */
void (*i2s_rx_callback)(void);          /* I2S RX DMA�ص�����ָ�� */
 
/**
 * @brief       I2S TX DMA �жϷ�����
 * @param       ��
 * @retval      ��
 */
void I2S_TX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    
    if (I2S_TX_DMASx_IS_TC())           /* I2S TX DMA �������? */
    {
        I2S_TX_DMASx_CLR_TC();          /* ��� I2S TX DMA ��������жϱ�־ */
        i2s_tx_callback();              /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */
    }
    
    OSIntExit();
}

/**
 * @brief       I2Sext RX DMA �жϷ�����
 * @param       ��
 * @retval      ��
 */
void I2SEXT_RX_DMASx_IRQHandler(void)
{
    OSIntEnter();
    
    if (I2SEXT_RX_DMASx_IS_TC())        /* I2Sext RX DMA �������? */
    {
        I2SEXT_RX_DMASx_CLR_TC();       /* ��� I2Sext RX DMA ��������жϱ�־ */
        i2s_rx_callback();              /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */
    }
    
    OSIntExit();
}

/**
 * @brief       I2S��ʼ����
 * @param       ��
 * @retval      ��
 */
void i2s_play_start(void)
{
    I2S_TX_DMASx->CR |= 1 << 0;;        /* ����I2S TX DMA���� */
}

/**
 * @brief       I2Sֹͣ����
 * @param       ��
 * @retval      ��
 */
void i2s_play_stop(void)
{
    I2S_TX_DMASx->CR &= ~(1 << 0);      /* �ر�I2S TX DMA���� */
}

/**
 * @brief       I2S��ʼ¼��
 * @param       ��
 * @retval      ��
 */
void i2s_rec_start(void)
{
    I2SEXT_RX_DMASx->CR |= 1 << 0;;     /* ����I2S RX DMA���� */
}

/**
 * @brief       I2Sֹͣ¼��
 * @param       ��
 * @retval      ��
 */
void i2s_rec_stop(void)
{
    I2SEXT_RX_DMASx->CR &= ~(1 << 0);   /* �ر�I2S RX DMA���� */
}

























