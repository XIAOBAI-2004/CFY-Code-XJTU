/**
 ****************************************************************************************************
 * @file        sys_arch.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-02
 * @brief       lwip-ϵͳ��ӿ����� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.1 20221202
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
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
 
#define MAX_QUEUES              10              /* ��Ϣ��������� */
#define MAX_QUEUE_ENTRIES       20              /* ÿ����Ϣ����Ĵ�С */

/* LWIP��Ϣ����ṹ�� */
typedef struct {
    OS_EVENT*   pQ;                             /* UCOS��ָ���¼����ƿ��ָ�� */
    void*       pvQEntries[MAX_QUEUE_ENTRIES];  /* ��Ϣ���� MAX_QUEUE_ENTRIES��Ϣ�����������Ϣ�� */
} TQ_DESCR, *PQ_DESCR;


typedef OS_EVENT *sys_sem_t;    /* LWIPʹ�õ��ź��� */
typedef OS_EVENT *sys_mutex_t;  /* LWIPʹ�õĻ����ź��� */
typedef PQ_DESCR sys_mbox_t;    /* LWIPʹ�õ���Ϣ����,��ʵ����UCOS�е���Ϣ���� */
typedef INT8U sys_thread_t;     /* �߳�ID,Ҳ�����������ȼ� */
typedef INT8U sys_prot_t;

#endif 






























