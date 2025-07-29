/**
 ****************************************************************************************************
 * @file        ethernet.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-01
 * @brief       网络PHY底层 驱动代码
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
 * V1.0 20221201
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/ETHERNET/stm32f4x7_eth.h"
#include "./BSP/ETHERNET/ethernet.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "ucos_ii.h"


ETH_DMADESCTypeDef *DMARxDscrTab;   /* 以太网DMA接收描述符数据结构体指针 */
ETH_DMADESCTypeDef *DMATxDscrTab;   /* 以太网DMA发送描述符数据结构体指针 */
uint8_t *Rx_Buff;                   /* 以太网底层驱动接收buffers指针 */
uint8_t *Tx_Buff;                   /* 以太网底层驱动发送buffers指针 */


/**
 * @brief       网络PHY初始化
 * @param       无
 * @retval      0, 成功
 *              其他, 失败
 */
uint8_t ethernet_init(void)
{
    uint8_t rval = 0;
    
    /* ETH IO接口初始化 */
    RCC->AHB1ENR |= 1 << 0;     /* 使能PORTA时钟 */
    RCC->AHB1ENR |= 1 << 2;     /* 使能PORTC时钟 */
    RCC->AHB1ENR |= 1 << 3;     /* 使能PORTD时钟 */
    RCC->AHB1ENR |= 1 << 6;     /* 使能PORTG时钟 */
    RCC->APB2ENR |= 1 << 14;    /* 使能SYSCFG时钟 */
    SYSCFG->PMC |= 1 << 23;     /* 使用RMII PHY接口 */

    /* GPIO工作模式设置 */
    sys_gpio_set(GPIOD, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* PD3 推挽输出 */

    sys_gpio_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA1,2,7 复用输出 */
    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PC1,4,5 复用输出 */
    
    sys_gpio_set(GPIOG, SYS_GPIO_PIN11 | SYS_GPIO_PIN13 | SYS_GPIO_PIN14,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PG11,13,14 复用输出 */

    /* GPIO工作复用功能功能设置 */
    sys_gpio_af_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7, 11);   /* PA1,2,7 AF11 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5, 11);   /* PC1,4,5 AF11 */
    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN11 | SYS_GPIO_PIN13 | SYS_GPIO_PIN14, 11);/* PG11,13,14 AF11 */

    ETHERNET_RST(0);    /* 硬件复位PHY */
    delay_ms(50);
    ETHERNET_RST(1);    /* 复位结束 */
    
    sys_nvic_init(2, 0, ETH_IRQn, 2);   /* 配置ETH中的分组 */
    
    rval = ethernet_macdma_config();
    return !rval;       /* ETH的规则为:0,失败;1,成功;所以要取反一下 */
}

/**
 * @brief       得到PHY的速度
 * @param       无
 * @retval      0, 10M
 *              1, 100M
 */
uint8_t ethernet_get_speed(void)
{
    uint8_t speed;
    #if(PHY_TYPE == LAN8720) 
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 4);     /* 从LAN8720的31号寄存器中读取网络速度和双工模式 */
    #elif(PHY_TYPE == SR8201F)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 13);    /* 从SR8201F的0号寄存器中读取网络速度和双工模式 */
    #elif(PHY_TYPE == YT8512C)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 14);    /* 从YT8512C的17号寄存器中读取网络速度和双工模式 */
    #elif(PHY_TYPE == RTL8201)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 1);     /* 从RTL8201的16号寄存器中读取网络速度和双工模式 */
    #endif

    return speed;
}

/***********************以下部分为STM32F407网卡配置/接口函数***********************/

/**
 * @brief       初始化ETH MAC层及DMA配置
 * @param       无
 * @retval      ETH_ERROR, 发送失败(0)
 *              ETH_SUCCESS, 发送成功(1)
 */
uint8_t ethernet_macdma_config(void)
{
    uint8_t rval;
    ETH_InitTypeDef ETH_InitStructure;

    /* 使能以太网时钟 */
    RCC->AHB1ENR |= 7 << 25;    /* 使能ETH MAC/MAC_Tx/MAC_Rx时钟 */

    ETH_DeInit();               /* AHB总线重启以太网 */
    ETH_SoftwareReset();        /* 软件重启网络 */

    while (ETH_GetSoftwareResetStatus() == SET);/* 等待软件重启网络完成 */

    ETH_StructInit(&ETH_InitStructure);         /* 初始化网络为默认值 */

    /* /网络MAC参数设置 */
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;                     /* 开启网络自适应功能 */
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;                          /* 关闭反馈 */
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;                /* 关闭重传功能kp */
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;          /* 关闭自动去除PDA/CRC功能 */
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;                              /* 关闭接收所有的帧 */
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;   /* 允许接收所有广播帧 */
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;                    /* 关闭混合模式的地址过滤 */
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;        /* 对于组播地址使用完美地址过滤 */
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;            /* 对单播地址使用完美地址过滤 */
#ifdef CHECKSUM_BY_HARDWARE
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;                     /* 开启ipv4和TCP/UDP/ICMP的帧校验和卸载 */
#endif
    /* 当我们使用帧校验和卸载功能的时候，一定要使能存储转发模式,存储转发模式中要保证整个帧存储在FIFO中 */
    /* 这样MAC能插入/识别出帧校验值,当真校验正确的时候DMA就可以处理帧,否则就丢弃掉该帧 */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; /* 开启丢弃TCP/IP错误帧 */
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;             /* 开启接收数据的存储转发模式 */
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;           /* 开启发送数据的存储转发模式 */

    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;              /* 禁止转发错误帧 */
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;    /* 不转发过小的好帧 */
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;               /* 打开处理第二帧功能 */
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;             /* 开启DMA传输的地址对齐功能 */
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                               /* 开启固定突发功能 */
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;                   /* DMA发送的最大突发长度为32个节拍 */
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                   /* DMA接收的最大突发长度为32个节拍 */
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
    ETH_InitStructure.Sys_Clock_Freq = 168;                                                 /* 系统时钟频率为168Mhz */
    rval = ETH_Init(&ETH_InitStructure, ETHERNET_PHY_ADDRESS);                              /* 配置ETH */

    if (rval == ETH_SUCCESS) /* 配置成功 */
    {
        ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE); /* 使能以太网接收中断 */
    }

    return rval;
}
extern void lwip_packet_handler(void);  /* 在lwip_comm.c里面定义 */

/**
 * @brief       以太网中断服务函数
 * @param       无
 * @retval      无
 */
void ETH_IRQHandler(void)
{
    OSIntEnter();   /* 进入中断 */

    while (ETH_GetRxPktSize(DMARxDescToGet) != 0)
    {
        lwip_packet_handler();
    }

    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    
    OSIntExit();    /* 触发任务切换软中断 */
}

/**
 * @brief       接收一个网卡数据包
 * @param       无
 * @retval      网络数据包帧结构体
 */
FrameTypeDef ethernet_rx_packet(void)
{
    uint32_t framelength = 0;
    FrameTypeDef frame = {0, 0};

    /* 检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候) */
    if ((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET)
    {
        frame.length = ETH_ERROR;

        if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
        {
            ETH->DMASR = ETH_DMASR_RBUS;/* 清除ETH DMA的RBUS位 */
            ETH->DMARPDR = 0; /* 恢复DMA接收 */
        }

        return frame;/* 错误,OWN位被设置了 */
    }

    if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET))
    {
        framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4; /* 得到接收包帧长度(不包含4字节CRC) */
        frame.buffer = DMARxDescToGet->Buffer1Addr;/* 得到包数据所在的位置 */
    }
    else framelength = ETH_ERROR; /* 错误 */

    frame.length = framelength;
    frame.descriptor = DMARxDescToGet;
    
    /* 更新ETH DMA全局Rx描述符为下一个Rx描述符 */
    /* 为下一次buffer读取设置下一个DMA Rx描述符 */
    DMARxDescToGet = (ETH_DMADESCTypeDef *)(DMARxDescToGet->Buffer2NextDescAddr);
    return frame;
}

/**
 * @brief       接收一个网卡数据包
 * @param       frame_length    : 数据包长度
 * @retval      ETH_ERROR, 发送失败(0)
 *              ETH_SUCCESS, 发送成功(1)
 */
uint8_t ethernet_tx_packet(uint16_t frame_length)
{
    /* 检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候) */
    if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)return ETH_ERROR; /* 错误,OWN位被设置了 */

    DMATxDescToSet->ControlBufferSize = (frame_length & ETH_DMATxDesc_TBS1); /* 设置帧长度,bits[12:0] */
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS; /* 设置最后一个和第一个位段置位(1个描述符传输一帧) */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN; /* 设置Tx描述符的OWN位,buffer重归ETH DMA */

    if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET) /* 当Tx Buffer不可用位(TBUS)被设置的时候,重置它.恢复传输 */
    {
        ETH->DMASR = ETH_DMASR_TBUS;    /* 重置ETH DMA TBUS位 */
        ETH->DMATPDR = 0;               /* 恢复DMA发送 */
    }

    /* 更新ETH DMA全局Tx描述符为下一个Tx描述符 */
    /* 为下一次buffer发送设置下一个DMA Tx描述符 */
    DMATxDescToSet = (ETH_DMADESCTypeDef *)(DMATxDescToSet->Buffer2NextDescAddr);
    return ETH_SUCCESS;
}

/**
 * @brief       得到当前描述符的Tx buffer地址
 * @param       无
 * @retval      Tx buffer地址
 */
uint32_t ethernet_get_current_tx_buffer(void)
{
    return DMATxDescToSet->Buffer1Addr; /* 返回Tx buffer地址 */
}

/**
 * @brief       为ETH底层驱动申请内存
 * @param       无
 * @retval      0, 成功
 *              1, 失败
 */
uint8_t ethernet_mem_malloc(void)
{
    DMARxDscrTab = mymalloc(SRAMIN, ETH_RXBUFNB * sizeof(ETH_DMADESCTypeDef)); /* 申请内存 */
    DMATxDscrTab = mymalloc(SRAMIN, ETH_TXBUFNB * sizeof(ETH_DMADESCTypeDef)); /* 申请内存 */
    Rx_Buff = mymalloc(SRAMIN, ETH_RX_BUF_SIZE * ETH_RXBUFNB);  /* 申请内存 */
    Tx_Buff = mymalloc(SRAMIN, ETH_TX_BUF_SIZE * ETH_TXBUFNB);  /* 申请内存 */

    if (!DMARxDscrTab || !DMATxDscrTab || !Rx_Buff || !Tx_Buff)
    {
        ethernet_mem_free();
        return 1;   /* 申请失败 */
    }

    return 0;       /* 申请成功 */
}

/**
 * @brief       释放ETH 底层驱动申请的内存
 * @param       无
 * @retval      无
 */
void ethernet_mem_free(void)
{
    myfree(SRAMIN, DMARxDscrTab);   /* 释放内存 */
    myfree(SRAMIN, DMATxDscrTab);   /* 释放内存 */
    myfree(SRAMIN, Rx_Buff);        /* 释放内存 */
    myfree(SRAMIN, Tx_Buff);        /* 释放内存 */
}
























