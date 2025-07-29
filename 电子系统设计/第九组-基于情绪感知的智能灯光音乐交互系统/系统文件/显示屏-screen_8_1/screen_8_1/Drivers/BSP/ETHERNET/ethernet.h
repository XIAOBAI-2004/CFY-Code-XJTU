/**
 ****************************************************************************************************
 * @file        ethernet.h
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

#ifndef __ETHERNET_H
#define __ETHERNET_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/ETHERNET/stm32f4x7_eth.h"



#define ETHERNET_PHY_ADDRESS    0x00                                        /* ETHERNET PHY芯片地址 */ 
#define ETHERNET_RST(x)         sys_gpio_pin_set(GPIOD, SYS_GPIO_PIN3, x)   /* ETHERNET复位引脚 */


extern ETH_DMADESCTypeDef *DMARxDscrTab;            /* 以太网DMA接收描述符数据结构体指针 */ 
extern ETH_DMADESCTypeDef *DMATxDscrTab;            /* 以太网DMA发送描述符数据结构体指针 */ 
extern uint8_t *Rx_Buff;                            /* 以太网底层驱动接收buffers指针 */  
extern uint8_t *Tx_Buff;                            /* 以太网底层驱动发送buffers指针 */ 
extern ETH_DMADESCTypeDef  *DMATxDescToSet;         /* DMA发送描述符追踪指针 */ 
extern ETH_DMADESCTypeDef  *DMARxDescToGet;         /* DMA接收描述符追踪指针 */ 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;  /* DMA最后接收到的帧信息指针 */ 
 

uint8_t ethernet_init(void);
uint8_t ethernet_get_speed(void);

uint8_t ethernet_macdma_config(void);
FrameTypeDef ethernet_rx_packet(void);
uint8_t ethernet_tx_packet(uint16_t frame_length);
uint32_t ethernet_get_current_tx_buffer(void);
uint8_t ethernet_mem_malloc(void);
void ethernet_mem_free(void);

#endif 

