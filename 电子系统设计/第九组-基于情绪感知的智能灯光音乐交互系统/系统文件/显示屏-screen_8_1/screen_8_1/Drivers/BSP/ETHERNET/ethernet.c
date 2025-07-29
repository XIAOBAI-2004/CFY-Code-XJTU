/**
 ****************************************************************************************************
 * @file        ethernet.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-01
 * @brief       ����PHY�ײ� ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20221201
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/ETHERNET/stm32f4x7_eth.h"
#include "./BSP/ETHERNET/ethernet.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "ucos_ii.h"


ETH_DMADESCTypeDef *DMARxDscrTab;   /* ��̫��DMA�������������ݽṹ��ָ�� */
ETH_DMADESCTypeDef *DMATxDscrTab;   /* ��̫��DMA�������������ݽṹ��ָ�� */
uint8_t *Rx_Buff;                   /* ��̫���ײ���������buffersָ�� */
uint8_t *Tx_Buff;                   /* ��̫���ײ���������buffersָ�� */


/**
 * @brief       ����PHY��ʼ��
 * @param       ��
 * @retval      0, �ɹ�
 *              ����, ʧ��
 */
uint8_t ethernet_init(void)
{
    uint8_t rval = 0;
    
    /* ETH IO�ӿڳ�ʼ�� */
    RCC->AHB1ENR |= 1 << 0;     /* ʹ��PORTAʱ�� */
    RCC->AHB1ENR |= 1 << 2;     /* ʹ��PORTCʱ�� */
    RCC->AHB1ENR |= 1 << 3;     /* ʹ��PORTDʱ�� */
    RCC->AHB1ENR |= 1 << 6;     /* ʹ��PORTGʱ�� */
    RCC->APB2ENR |= 1 << 14;    /* ʹ��SYSCFGʱ�� */
    SYSCFG->PMC |= 1 << 23;     /* ʹ��RMII PHY�ӿ� */

    /* GPIO����ģʽ���� */
    sys_gpio_set(GPIOD, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* PD3 ������� */

    sys_gpio_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PA1,2,7 ������� */
    
    sys_gpio_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PC1,4,5 ������� */
    
    sys_gpio_set(GPIOG, SYS_GPIO_PIN11 | SYS_GPIO_PIN13 | SYS_GPIO_PIN14,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_NONE); /* PG11,13,14 ������� */

    /* GPIO�������ù��ܹ������� */
    sys_gpio_af_set(GPIOA, SYS_GPIO_PIN1 | SYS_GPIO_PIN2 | SYS_GPIO_PIN7, 11);   /* PA1,2,7 AF11 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN1 | SYS_GPIO_PIN4 | SYS_GPIO_PIN5, 11);   /* PC1,4,5 AF11 */
    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN11 | SYS_GPIO_PIN13 | SYS_GPIO_PIN14, 11);/* PG11,13,14 AF11 */

    ETHERNET_RST(0);    /* Ӳ����λPHY */
    delay_ms(50);
    ETHERNET_RST(1);    /* ��λ���� */
    
    sys_nvic_init(2, 0, ETH_IRQn, 2);   /* ����ETH�еķ��� */
    
    rval = ethernet_macdma_config();
    return !rval;       /* ETH�Ĺ���Ϊ:0,ʧ��;1,�ɹ�;����Ҫȡ��һ�� */
}

/**
 * @brief       �õ�PHY���ٶ�
 * @param       ��
 * @retval      0, 10M
 *              1, 100M
 */
uint8_t ethernet_get_speed(void)
{
    uint8_t speed;
    #if(PHY_TYPE == LAN8720) 
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 4);     /* ��LAN8720��31�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    #elif(PHY_TYPE == SR8201F)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 13);    /* ��SR8201F��0�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    #elif(PHY_TYPE == YT8512C)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 14);    /* ��YT8512C��17�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    #elif(PHY_TYPE == RTL8201)
    speed = ((ETH_ReadPHYRegister(0, PHY_SR) & PHY_SPEED_STATUS) >> 1);     /* ��RTL8201��16�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ */
    #endif

    return speed;
}

/***********************���²���ΪSTM32F407��������/�ӿں���***********************/

/**
 * @brief       ��ʼ��ETH MAC�㼰DMA����
 * @param       ��
 * @retval      ETH_ERROR, ����ʧ��(0)
 *              ETH_SUCCESS, ���ͳɹ�(1)
 */
uint8_t ethernet_macdma_config(void)
{
    uint8_t rval;
    ETH_InitTypeDef ETH_InitStructure;

    /* ʹ����̫��ʱ�� */
    RCC->AHB1ENR |= 7 << 25;    /* ʹ��ETH MAC/MAC_Tx/MAC_Rxʱ�� */

    ETH_DeInit();               /* AHB����������̫�� */
    ETH_SoftwareReset();        /* ����������� */

    while (ETH_GetSoftwareResetStatus() == SET);/* �ȴ��������������� */

    ETH_StructInit(&ETH_InitStructure);         /* ��ʼ������ΪĬ��ֵ */

    /* /����MAC�������� */
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;                     /* ������������Ӧ���� */
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;                          /* �رշ��� */
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;                /* �ر��ش�����kp */
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;          /* �ر��Զ�ȥ��PDA/CRC���� */
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;                              /* �رս������е�֡ */
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;   /* ����������й㲥֡ */
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;                    /* �رջ��ģʽ�ĵ�ַ���� */
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;        /* �����鲥��ַʹ��������ַ���� */
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;            /* �Ե�����ַʹ��������ַ���� */
#ifdef CHECKSUM_BY_HARDWARE
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;                     /* ����ipv4��TCP/UDP/ICMP��֡У���ж�� */
#endif
    /* ������ʹ��֡У���ж�ع��ܵ�ʱ��һ��Ҫʹ�ܴ洢ת��ģʽ,�洢ת��ģʽ��Ҫ��֤����֡�洢��FIFO�� */
    /* ����MAC�ܲ���/ʶ���֡У��ֵ,����У����ȷ��ʱ��DMA�Ϳ��Դ���֡,����Ͷ�������֡ */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; /* ��������TCP/IP����֡ */
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;             /* �����������ݵĴ洢ת��ģʽ */
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;           /* �����������ݵĴ洢ת��ģʽ */

    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;              /* ��ֹת������֡ */
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;    /* ��ת����С�ĺ�֡ */
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;               /* �򿪴���ڶ�֡���� */
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;             /* ����DMA����ĵ�ַ���빦�� */
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                               /* �����̶�ͻ������ */
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;                   /* DMA���͵����ͻ������Ϊ32������ */
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                   /* DMA���յ����ͻ������Ϊ32������ */
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
    ETH_InitStructure.Sys_Clock_Freq = 168;                                                 /* ϵͳʱ��Ƶ��Ϊ168Mhz */
    rval = ETH_Init(&ETH_InitStructure, ETHERNET_PHY_ADDRESS);                              /* ����ETH */

    if (rval == ETH_SUCCESS) /* ���óɹ� */
    {
        ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE); /* ʹ����̫�������ж� */
    }

    return rval;
}
extern void lwip_packet_handler(void);  /* ��lwip_comm.c���涨�� */

/**
 * @brief       ��̫���жϷ�����
 * @param       ��
 * @retval      ��
 */
void ETH_IRQHandler(void)
{
    OSIntEnter();   /* �����ж� */

    while (ETH_GetRxPktSize(DMARxDescToGet) != 0)
    {
        lwip_packet_handler();
    }

    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    
    OSIntExit();    /* ���������л����ж� */
}

/**
 * @brief       ����һ���������ݰ�
 * @param       ��
 * @retval      �������ݰ�֡�ṹ��
 */
FrameTypeDef ethernet_rx_packet(void)
{
    uint32_t framelength = 0;
    FrameTypeDef frame = {0, 0};

    /* ��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��) */
    if ((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET)
    {
        frame.length = ETH_ERROR;

        if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
        {
            ETH->DMASR = ETH_DMASR_RBUS;/* ���ETH DMA��RBUSλ */
            ETH->DMARPDR = 0; /* �ָ�DMA���� */
        }

        return frame;/* ����,OWNλ�������� */
    }

    if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET))
    {
        framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4; /* �õ����հ�֡����(������4�ֽ�CRC) */
        frame.buffer = DMARxDescToGet->Buffer1Addr;/* �õ����������ڵ�λ�� */
    }
    else framelength = ETH_ERROR; /* ���� */

    frame.length = framelength;
    frame.descriptor = DMARxDescToGet;
    
    /* ����ETH DMAȫ��Rx������Ϊ��һ��Rx������ */
    /* Ϊ��һ��buffer��ȡ������һ��DMA Rx������ */
    DMARxDescToGet = (ETH_DMADESCTypeDef *)(DMARxDescToGet->Buffer2NextDescAddr);
    return frame;
}

/**
 * @brief       ����һ���������ݰ�
 * @param       frame_length    : ���ݰ�����
 * @retval      ETH_ERROR, ����ʧ��(0)
 *              ETH_SUCCESS, ���ͳɹ�(1)
 */
uint8_t ethernet_tx_packet(uint16_t frame_length)
{
    /* ��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��) */
    if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (uint32_t)RESET)return ETH_ERROR; /* ����,OWNλ�������� */

    DMATxDescToSet->ControlBufferSize = (frame_length & ETH_DMATxDesc_TBS1); /* ����֡����,bits[12:0] */
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS; /* �������һ���͵�һ��λ����λ(1������������һ֡) */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN; /* ����Tx��������OWNλ,buffer�ع�ETH DMA */

    if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET) /* ��Tx Buffer������λ(TBUS)�����õ�ʱ��,������.�ָ����� */
    {
        ETH->DMASR = ETH_DMASR_TBUS;    /* ����ETH DMA TBUSλ */
        ETH->DMATPDR = 0;               /* �ָ�DMA���� */
    }

    /* ����ETH DMAȫ��Tx������Ϊ��һ��Tx������ */
    /* Ϊ��һ��buffer����������һ��DMA Tx������ */
    DMATxDescToSet = (ETH_DMADESCTypeDef *)(DMATxDescToSet->Buffer2NextDescAddr);
    return ETH_SUCCESS;
}

/**
 * @brief       �õ���ǰ��������Tx buffer��ַ
 * @param       ��
 * @retval      Tx buffer��ַ
 */
uint32_t ethernet_get_current_tx_buffer(void)
{
    return DMATxDescToSet->Buffer1Addr; /* ����Tx buffer��ַ */
}

/**
 * @brief       ΪETH�ײ����������ڴ�
 * @param       ��
 * @retval      0, �ɹ�
 *              1, ʧ��
 */
uint8_t ethernet_mem_malloc(void)
{
    DMARxDscrTab = mymalloc(SRAMIN, ETH_RXBUFNB * sizeof(ETH_DMADESCTypeDef)); /* �����ڴ� */
    DMATxDscrTab = mymalloc(SRAMIN, ETH_TXBUFNB * sizeof(ETH_DMADESCTypeDef)); /* �����ڴ� */
    Rx_Buff = mymalloc(SRAMIN, ETH_RX_BUF_SIZE * ETH_RXBUFNB);  /* �����ڴ� */
    Tx_Buff = mymalloc(SRAMIN, ETH_TX_BUF_SIZE * ETH_TXBUFNB);  /* �����ڴ� */

    if (!DMARxDscrTab || !DMATxDscrTab || !Rx_Buff || !Tx_Buff)
    {
        ethernet_mem_free();
        return 1;   /* ����ʧ�� */
    }

    return 0;       /* ����ɹ� */
}

/**
 * @brief       �ͷ�ETH �ײ�����������ڴ�
 * @param       ��
 * @retval      ��
 */
void ethernet_mem_free(void)
{
    myfree(SRAMIN, DMARxDscrTab);   /* �ͷ��ڴ� */
    myfree(SRAMIN, DMATxDscrTab);   /* �ͷ��ڴ� */
    myfree(SRAMIN, Rx_Buff);        /* �ͷ��ڴ� */
    myfree(SRAMIN, Tx_Buff);        /* �ͷ��ڴ� */
}
























