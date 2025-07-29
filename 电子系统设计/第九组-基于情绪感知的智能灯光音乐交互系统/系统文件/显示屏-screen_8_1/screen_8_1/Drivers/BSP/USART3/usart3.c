/****************************************************************************************************
 * @file        usart3.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-10-29
 * @brief       串口3 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************/
#include "./BSP/USART3/usart3.h"
#include "./BSP/TIMER/timer.h"
#include "./MALLOC/malloc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ucos_ii.h"

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

/* 不使用半主机模式，至少需要重定义_ttywrch_sys_exit_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
//volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
//volatile uint8_t tx_head = 0;
//volatile uint8_t tx_tail = 0;

//void usart3_send_char(uint8_t ch)
//{
//    while (((tx_head + 1) % TX_BUFFER_SIZE) == tx_tail);  // 等待缓冲区有空间
//    tx_buffer[tx_head] = ch;
//    tx_head = (tx_head + 1) % TX_BUFFER_SIZE;
//}

//void usart3_flush(void)
//{
//    if (tx_head != tx_tail)
//    {
//        USART3->DR = tx_buffer[tx_tail];
//        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
//    }
//}

int fputc(int ch, FILE *f)
{
    while ((USART3->SR & USART_SR_TXE) == 0);     /* 等待上一个字符发送完成 */
    USART3->DR = (uint8_t)ch;                     /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/******************************************************************************************/

/* 串口接收缓存区 */
uint8_t g_usart3_rx_buf[USART3_MAX_RECV_LEN];       /* 接收缓冲,最大USART3_MAX_RECV_LEN个字节 */
volatile uint16_t g_usart3_rx_sta = 0;

volatile uint8_t Serial_RxFlag = 0;
volatile char Serial_RxPacket[100];				
/**
 * @brief       串口3中断服务函数
 * @param       无
 * @retval      无
 */


// 命令字符串数组，顺序必须与枚举一致
const char *cmd_str[] = {
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

extern OS_FLAG_GRP *voice_event; // 事件控制块

void USART3_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint16_t pRxPacket = 0;
    uint8_t RxData;
    INT8U err;
    static voice_cmd_t static_cmd;  // 使用静态变量避免动态分配

    OSIntEnter();
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();

    if (USART3->SR & (1 << 5)) {
        RxData = USART3->DR;

        switch (RxState) {
            case 0: // 等待包头'@'
                if (RxData == '@' && Serial_RxFlag == 0) {
                    RxState = 1;
                    pRxPacket = 0;
                    g_usart3_rx_sta = 0;
									  memset((void*)Serial_RxPacket, 0, USART3_MAX_RECV_LEN); // 添加类型转换TIM7->CNT = 0;
                    TIM7->CR1 |= (1 << 0);
                }
                break;

            case 1: // 接收数据内容
                if (RxData == '%') {
                    RxState = 2; // 进入等待包尾状态
                } else {
                    if (pRxPacket < USART3_MAX_RECV_LEN - 1) {
                        Serial_RxPacket[pRxPacket++] = RxData;
                    } else {
                        RxState = 0;
                        TIM7->CR1 &= ~(1 << 0);
                        g_usart3_rx_sta |= (1 << 15);
                    }
                    TIM7->CNT = 0;
                }
                break;

            case 2: // 验证包尾'\n'
                if (RxData == '#') {
                    Serial_RxPacket[pRxPacket] = '\0'; // 正确结束字符串
									  
									u3_printf("Received full packet: %s\r\n", Serial_RxPacket);
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
									
									
                    for (int i = 0; i < sizeof(cmd_str)/sizeof(char*); i++) {
                        if (strstr((char *)Serial_RxPacket, cmd_str[i])) {
													  
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
                TIM7->CR1 &= ~(1 << 0);
                break;
        }

        USART3->SR &= ~(1 << 5);
    }

    OSIntExit();
    OS_EXIT_CRITICAL();
}
	

/**
 * @brief       串口3初始化函数
 * @param       sclk: 串口X的时钟源频率(单位: MHz)
 *              串口1 的时钟源来自: PCLK2 = 72Mhz
 *              串口2 - 5 的时钟源来自: PCLK1 = 36Mhz
 * @note        注意: 必须设置正确的sclk, 否则串口波特率就会设置异常.
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @retval      无
 */


void usart3_init(uint32_t sclk, uint32_t baudrate)
{
    USART3_TX_GPIO_CLK_ENABLE();    /* 使能串口TX脚时钟 */
    USART3_RX_GPIO_CLK_ENABLE();    /* 使能串口RX脚时钟 */
    
   /* TX引脚（PB10）配置为复用推挽输出 */
    sys_gpio_set(USART3_TX_GPIO_PORT, USART3_TX_GPIO_PIN,
                SYS_GPIO_MODE_AF,          // 复用模式
                SYS_GPIO_OTYPE_PP,         // 推挽输出
                SYS_GPIO_SPEED_HIGH,       // 高速
                SYS_GPIO_PUPD_PU);         // 上拉

    /* RX引脚（PB11）配置为复用输入 */
    sys_gpio_set(USART3_RX_GPIO_PORT, USART3_RX_GPIO_PIN,
                SYS_GPIO_MODE_AF,          // 复用模式
                SYS_GPIO_OTYPE_PP,
                SYS_GPIO_SPEED_HIGH,
                SYS_GPIO_PUPD_PU);
	
    sys_gpio_af_set(USART3_TX_GPIO_PORT, USART3_TX_GPIO_PIN, 7);    /* TX脚 复用功能选择, 必须设置正确 */
    sys_gpio_af_set(USART3_RX_GPIO_PORT, USART3_RX_GPIO_PIN, 7);    /* RX脚 复用功能选择, 必须设置正确 */

    RCC->APB1ENR |= 1 << 18;        /* 使能串口3时钟 */
    RCC->APB1RSTR |= 1 << 18;       /* 复位串口3 */
    RCC->APB1RSTR &= ~(1 << 18);    /* 停止复位 */
    
    /* 波特率设置 */
    USART3->BRR = (sclk * 1000000) / (baudrate);
    USART3->CR1 |= 0X200C;          /* 1位停止,无校验位 */
    
    /* 使能接收中断 */
    USART3->CR1 |= 1 << 5;          /* 接收缓冲区非空中断使能 */
    
    sys_nvic_init(0, 1, USART3_IRQn, 2); /* 组2 */
   
    tim7_int_init(99, 8400 - 1);    /* 10ms中断 */
    TIM7->CR1 &= ~(1 << 0);         /* 关闭定时器7 */
    g_usart3_rx_sta = 0;            /* 清零 */
}

/**
 * @brief       串口3,printf 函数
 * @param       fmt     : 格式化参数
 * @param       ...     : 可变参数
 * @retval      无
 */
void u3_printf(char *fmt, ...)
{
    uint16_t i, j;
    uint8_t *pbuf;
    va_list ap;
    
    pbuf = mymalloc(SRAMIN, USART3_MAX_SEND_LEN);   /* 申请内存 */

    if (!pbuf)  /* 内存申请失败 */
    {
        printf("u3 malloc error\r\n");
        return ;
    }

    va_start(ap, fmt);
    vsprintf((char *)pbuf, fmt, ap);
    va_end(ap);
    i = strlen((const char *)pbuf); /* 此次发送数据的长度 */

    for (j = 0; j < i; j++)         /* 循环发送数据 */
    {
        while ((USART3->SR & 0X40) == 0);   /* 循环发送,直到发送完毕 */

        USART3->DR = pbuf[j];
    }

    myfree(SRAMIN, pbuf);           /* 释放内存 */
}

