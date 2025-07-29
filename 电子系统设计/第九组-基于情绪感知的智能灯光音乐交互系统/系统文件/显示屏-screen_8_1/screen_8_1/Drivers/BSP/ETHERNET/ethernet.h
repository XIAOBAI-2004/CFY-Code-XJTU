/**
 ****************************************************************************************************
 * @file        ethernet.h
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

#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/ETHERNET/stm32f4x7_eth.h"



#define ETHERNET_PHY_ADDRESS    0x00                                        /* ETHERNET PHYоƬ��ַ */ 
#define ETHERNET_RST(x)         sys_gpio_pin_set(GPIOD, SYS_GPIO_PIN3, x)   /* ETHERNET��λ���� */


extern ETH_DMADESCTypeDef *DMARxDscrTab;            /* ��̫��DMA�������������ݽṹ��ָ�� */ 
extern ETH_DMADESCTypeDef *DMATxDscrTab;            /* ��̫��DMA�������������ݽṹ��ָ�� */ 
extern uint8_t *Rx_Buff;                            /* ��̫���ײ���������buffersָ�� */  
extern uint8_t *Tx_Buff;                            /* ��̫���ײ���������buffersָ�� */ 
extern ETH_DMADESCTypeDef  *DMATxDescToSet;         /* DMA����������׷��ָ�� */ 
extern ETH_DMADESCTypeDef  *DMARxDescToGet;         /* DMA����������׷��ָ�� */ 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;  /* DMA�����յ���֡��Ϣָ�� */ 
 

uint8_t ethernet_init(void);
uint8_t ethernet_get_speed(void);

uint8_t ethernet_macdma_config(void);
FrameTypeDef ethernet_rx_packet(void);
uint8_t ethernet_tx_packet(uint16_t frame_length);
uint32_t ethernet_get_current_tx_buffer(void);
uint8_t ethernet_mem_malloc(void);
void ethernet_mem_free(void);

#endif 

