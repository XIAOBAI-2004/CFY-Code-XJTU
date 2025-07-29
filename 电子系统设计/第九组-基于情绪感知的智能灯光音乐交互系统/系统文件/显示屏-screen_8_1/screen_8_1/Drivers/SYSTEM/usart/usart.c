/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       串口3 驱动代码
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
 * V1.0 20221202
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

#include "./BSP/TIMER/timer.h"
#include "./MALLOC/malloc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ucos_ii.h"

#include "./BSP/USART3/usart3.h"

#include "os.h"   /* os 使用 */
#include "audioplay.h"


/* 如果使用os,则包括下面的头文件即可. */
#if SYS_SUPPORT_OS
#include "os.h"   /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)            /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");  /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");    /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */

/* 定义_sys_exit()以避免使用半主机模式 */


/* FILE 在 stdio.h里面定义. */


/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */

#endif
/******************************************************************************************/


#if USART_EN_RX     /* 如果使能了接收 */

/* 接收缓冲, 最大USART_REC_LEN个字节. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  接收状态
 *  bit15，      接收完成标志
 *  bit14，      接收到0x0d
 *  bit13~0，    接收到的有效字节数目
*/
uint16_t g_usart_rx_sta = 0;

/**
 * @brief       串口X中断服务函数
 * @param       无
 * @retval      无
 */
 
 
extern volatile uint8_t Serial_RxFlag;
extern volatile char Serial_RxPacket[100];		




// 命令字符串数组，顺序必须与枚举一致
// 命令字符串数组，顺序必须与枚举一致
const char *cmd_str1[] = {
    "bofang",    // 播放
    "zanting",   // 暂停
    "shangyishou", // 上一曲
    "xiayishou" // 下一曲
//	"liushui",    // 流水
//	"qisejianbian", // 七色渐变
//	"shanshuo",    //  闪烁
//	"xunhuan",    //   循环	
//	"kaideng",    //开灯
//	"guandeng",    //关灯
//	"kaixin",     //   开心
//	"fennu",      //  愤怒
//	"beishang",   //   悲伤
//	"kuaile",     //   快乐
//	"fuza"       //   复杂
};



/**
 * @brief       串口3中断服务函数
 * @param       无
 * @retval      无
 */




extern OS_FLAG_GRP *voice_event; // 事件控制块

void USART_UX_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint16_t pRxPacket = 0;
    uint8_t RxData;
    INT8U err;
    static voice_cmd_t static_cmd;  // 使用静态变量避免动态分配

    OSIntEnter();
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
	
    if (USART_UX->SR & (1 << 5)) {
        RxData = USART_UX->DR;

        switch (RxState) {
            case 0: // 等待包头'@'
                if (RxData == '@' && Serial_RxFlag == 0) {
                    RxState = 1;
                    pRxPacket = 0;
                    g_usart_rx_sta = 0;
									  memset((void*)Serial_RxPacket, 0, USART_REC_LEN); // 添加类型转换TIM7->CNT = 0;
                  //  TIM7->CR1 |= (1 << 0);
                }
                break;

            case 1: // 接收数据内容
                if (RxData == '%') {
                    RxState = 2; // 进入等待包尾状态
                } else {
                    if (pRxPacket < USART_REC_LEN - 1) {
                        Serial_RxPacket[pRxPacket++] = RxData;
                    } else {
                        RxState = 0;
                        //TIM7->CR1 &= ~(1 << 0);
                        g_usart_rx_sta |= (1 << 15);
                    }
                   // TIM7->CNT = 0;
                }
                break;

            case 2: // 验证包尾'\n'
                if (RxData == '#') {
                    Serial_RxPacket[pRxPacket] = '\0'; // 正确结束字符串
									  
									printf("Received full packet: %s\r\n", Serial_RxPacket);
									/* 处理播放指定歌曲命令 */
         /* 处理播放指定歌曲命令 */
        if (strncmp((const char *)Serial_RxPacket, "SONG:", 5) == 0) {
            voice_cmd_ex_t *cmd = mymalloc(SRAMIN, sizeof(voice_cmd_ex_t));
            if(cmd) {
                cmd->cmd_type = VOICE_CMD_PLAY_SONG;
                uint8_t *song_start = Serial_RxPacket + 5;
                
                /* 清理可能的换行/回车符 */
                uint8_t *p = song_start;
                while(*p && *p != '\r' && *p != '\n') p++;
                *p = '\0';
                
                /* 直接存储UTF-8字符串 */
                uint8_t len = strlen((const char*)song_start);
                len = (len > MAX_SONG_NAME_LEN-1) ? MAX_SONG_NAME_LEN-1 : len;
                memcpy(cmd->song_name, song_start, len);
                cmd->song_name[len] = '\0';
                
                OSQPost(g_voice_q, cmd);
                OSFlagPost(voice_event, 0x01, OS_FLAG_SET, &err);
            }
        }
        else {
									
									
                    for (int i = 0; i < sizeof(cmd_str1)/sizeof(char*); i++) {
                        if (strstr((char *)Serial_RxPacket, cmd_str1[i])) {
													  
                            static_cmd = (voice_cmd_t)(i+1);  // 使用静态变量
                            OSQPostOpt(g_voice_q, &static_cmd, OS_POST_OPT_FRONT);
                            OSFlagPost(voice_event, 0x01, OS_FLAG_SET, &err);
                            Serial_RxFlag = 1;  // 正确设置标志
													  
                            break;
                        }
                    }
                } 
//				else {
//                    // 包尾错误，丢弃数据
//                    g_usart3_rx_sta |= (1 << 15);
//                }
							}
                RxState = 0;
                pRxPacket = 0;
               // TIM7->CR1 &= ~(1 << 0);
                break;
        }

        USART_UX->SR &= ~(1 << 5);
    }

    OSIntExit();
    OS_EXIT_CRITICAL();
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
//void USART_UX_IRQHandler(void)
//{
//    uint8_t rxdata;
//#if SYS_SUPPORT_OS  /* 如果SYS_SUPPORT_OS为真，则需要支持OS. */
//    OSIntEnter();
//#endif

//    if (USART_UX->SR & (1 << 5))                /* 接收到数据 */
//    {
//        rxdata = USART_UX->DR;

//        if ((g_usart_rx_sta & 0x8000) == 0)     /* 接收未完成? */
//        {
//            if (g_usart_rx_sta & 0x4000)        /* 接收到了0x0d? */
//            {
//                if (rxdata != 0x0a)             /* 接收到了0x0a? (必须先接收到到0x0d,才检查0x0a) */
//                {
//                    g_usart_rx_sta = 0;         /* 接收错误, 重新开始 */
//                }
//                else
//                {
//                    g_usart_rx_sta |= 0x8000;   /* 收到了0x0a,标记接收完成了 */
//                }
//            }
//            else      /* 还没收到0x0d */
//            {
//                if (rxdata == 0x0d)
//                {
//                    g_usart_rx_sta |= 0x4000;   /* 标记接收到了 0x0d */
//                }
//                else
//                {
//                    g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = rxdata;   /* 存储数据到 g_usart_rx_buf */
//                    g_usart_rx_sta++;

//                    if (g_usart_rx_sta > (USART_REC_LEN - 1))g_usart_rx_sta = 0;/* 接收数据溢出, 重新开始接收 */
//                }
//            }
//        }
//    }

//#if SYS_SUPPORT_OS  /* 如果SYS_SUPPORT_OS为真，则需要支持OS. */
//    OSIntExit();
//#endif
//}
#endif

/**
 * @brief       串口X初始化函数
 * @param       sclk: 串口X的时钟源频率(单位: MHz)
 *              串口1 和 串口6 的时钟源来自: rcc_pclk2 = 84Mhz
 *              串口2 - 5 / 7 / 8 的时钟源来自: rcc_pclk1 = 42Mhz
 * @note        注意: 必须设置正确的sclk, 否则串口波特率就会设置异常.
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @retval      无
 */
void usart_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
    /* IO 及 时钟配置 */
    USART_TX_GPIO_CLK_ENABLE(); /* 使能串口TX脚时钟 */
    USART_RX_GPIO_CLK_ENABLE(); /* 使能串口RX脚时钟 */
    USART_UX_CLK_ENABLE();      /* 使能串口时钟 */

    sys_gpio_set(USART_TX_GPIO_PORT, USART_TX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* 串口TX脚 模式设置 */

    sys_gpio_set(USART_RX_GPIO_PORT, USART_RX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* 串口RX脚 模式设置 */

    sys_gpio_af_set(GPIOA, USART_TX_GPIO_PIN, USART_TX_GPIO_AF);    /* TX脚 复用功能选择, 必须设置正确 */
    sys_gpio_af_set(GPIOA, USART_RX_GPIO_PIN, USART_RX_GPIO_AF);    /* RX脚 复用功能选择, 必须设置正确 */

    temp = (sclk * 1000000 + baudrate / 2) / baudrate;              /* 得到USARTDIV@OVER8 = 0, 采用四舍五入计算 */
    /* 波特率设置 */
    USART_UX->BRR = temp;       /* 波特率设置@OVER8 = 0 */
    USART_UX->CR1 = 0;          /* 清零CR1寄存器 */
    USART_UX->CR1 |= 0 << 12;   /* 设置M = 0, 选择8位字长 */
    USART_UX->CR1 |= 0 << 15;   /* 设置OVER8 = 0, 16倍过采样 */
    USART_UX->CR1 |= 1 << 3;    /* 串口发送使能 */
#if USART_EN_RX  /* 如果使能了接收 */
    /* 使能接收中断 */
    USART_UX->CR1 |= 1 << 2;    /* 串口接收使能 */
    USART_UX->CR1 |= 1 << 5;    /* 接收缓冲区非空中断使能 */
    sys_nvic_init(3, 3, USART_UX_IRQn, 2); /* 组2，最低优先级 */
#endif
    USART_UX->CR1 |= 1 << 13;   /* 串口使能 */
}














