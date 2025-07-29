#ifndef __STM32F4x7_ETH_CONF_H
#define __STM32F4x7_ETH_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"

#define USE_ENHANCED_DMA_DESCRIPTORS

//#define USE_Delay

#ifdef USE_Delay
  #include "main.h"                /* Header file where the Delay function prototype is exported */  
  #define _eth_delay_    Delay     /* User can provide more timing precise _eth_delay_ function */
#else
  #define _eth_delay_    ETH_Delay /* Default _eth_delay_ function with less precise timing */
#endif


#ifdef  CUSTOM_DRIVER_BUFFERS_CONFIG
/* Redefinition of the Ethernet driver buffers size and count */   
 #define ETH_RX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for receive */
 #define ETH_TX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for transmit */
 #define ETH_RXBUFNB        20                  /* 20 Rx buffers of size ETH_RX_BUF_SIZE */
 #define ETH_TXBUFNB        5                   /* 5  Tx buffers of size ETH_TX_BUF_SIZE */
#endif


/* PHY配置块 **************************************************/
/* PHY复位延时*/ 
#define PHY_RESET_DELAY    ((uint32_t)0x000FFFFF) 

/* PHY配置延时*/ 
#define PHY_CONFIG_DELAY   ((uint32_t)0x00FFFFFF)


//速度和双工类型的的值,用户需要根据自己的PHY芯片来设置

#define LAN8720                          0
#define SR8201F                          1
#define YT8512C                          2
#define RTL8201                          3
#define PHY_TYPE                         YT8512C


#if(PHY_TYPE == LAN8720) 
#define PHY_SR                           ((uint16_t)0x1F)                       /*!< tranceiver status register */
#define PHY_SPEED_STATUS                 ((uint16_t)0x0004)                     /*!< configured information of speed: 100Mbit/s */
#define PHY_DUPLEX_STATUS                ((uint16_t)0x0010)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == SR8201F)
#define PHY_SR                           ((uint16_t)0x00)                       /*!< tranceiver status register */
#define PHY_SPEED_STATUS                 ((uint16_t)0x2020)                     /*!< configured information of speed: 100Mbit/s */
#define PHY_DUPLEX_STATUS                ((uint16_t)0x0100)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == YT8512C)
#define PHY_SR                           ((uint16_t)0x11)                       /*!< tranceiver status register */
#define PHY_SPEED_STATUS                 ((uint16_t)0xC000)                     /*!< configured information of speed: 100Mbit/s */
#define PHY_DUPLEX_STATUS                ((uint16_t)0x2000)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == RTL8201)
#define PHY_SR                           ((uint16_t)0x10)                       /*!< tranceiver status register */
#define PHY_SPEED_STATUS                 ((uint16_t)0x0022)                     /*!< configured information of speed: 100Mbit/s */
#define PHY_DUPLEX_STATUS                ((uint16_t)0x0004)                     /*!< configured information of duplex: full-duplex */
#endif /* PHY_TYPE */



#ifdef __cplusplus
}
#endif

#endif 


