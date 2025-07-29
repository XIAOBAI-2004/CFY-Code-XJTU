/**
 ****************************************************************************************************
 * @file        sys_arch.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       lwip-系统层接口驱动 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.1 20221202
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__ 
#include "arch/cc.h"
#include "os.h"


#ifdef SYS_ARCH_GLOBALS
#define SYS_ARCH_EXT
#else
#define SYS_ARCH_EXT extern
#endif
 
#define MAX_QUEUES              10              /* 消息邮箱的数量 */
#define MAX_QUEUE_ENTRIES       20              /* 每个消息邮箱的大小 */

/* LWIP消息邮箱结构体 */
typedef struct {
    OS_EVENT*   pQ;                             /* UCOS中指向事件控制块的指针 */
    void*       pvQEntries[MAX_QUEUE_ENTRIES];  /* 消息队列 MAX_QUEUE_ENTRIES消息队列中最多消息数 */
} TQ_DESCR, *PQ_DESCR;


typedef OS_EVENT *sys_sem_t;    /* LWIP使用的信号量 */
typedef OS_EVENT *sys_mutex_t;  /* LWIP使用的互斥信号量 */
typedef PQ_DESCR sys_mbox_t;    /* LWIP使用的消息邮箱,其实就是UCOS中的消息队列 */
typedef INT8U sys_thread_t;     /* 线程ID,也就是任务优先级 */
typedef INT8U sys_prot_t;

#endif 






























