/**
 ****************************************************************************************************
 * @file        sys_arch.c
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
 * V1.0 20221202
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "os.h"
#include "arch/sys_arch.h"


/**
 * ����Ϣָ��Ϊ��ʱ,ָ��һ������pvNullPointer��ָ���ֵ.
 * ��UCOS�����OSQPost()�е�msg==NULL�᷵��һ��OS_ERR_POST_NULL
 * ����,����lwip�л����sys_mbox_post(mbox,NULL)����һ������Ϣ,����
 * �ڱ������а�NULL���һ������ָ��0Xffffffff
 */
const void *const pvNullPointer = (mem_ptr_t *)0xffffffff;

/**
 * @brief       ����һ����Ϣ����
 * @param       mbox            : ��Ϣ����
 * @param       size            : �����С
 * @retval      ERR_OK,�����ɹ�; ����,����ʧ��;
 */
err_t sys_mbox_new( sys_mbox_t *mbox, int size)
{
    (*mbox) = mymalloc(SRAMIN, sizeof(TQ_DESCR));   /* Ϊ��Ϣ���������ڴ� */
    my_mem_set((*mbox), 0, sizeof(TQ_DESCR));       /* ���mbox���ڴ� */

    if (*mbox)  /* �ڴ����ɹ� */
    {
        if (size > MAX_QUEUE_ENTRIES)size = MAX_QUEUE_ENTRIES;      /* ��Ϣ�����������MAX_QUEUE_ENTRIES��Ϣ��Ŀ */

        (*mbox)->pQ = OSQCreate(&((*mbox)->pvQEntries[0]), size);   /* ʹ��UCOS����һ����Ϣ���� */
        LWIP_ASSERT("OSQCreate", (*mbox)->pQ != NULL);

        if ((*mbox)->pQ != NULL)return ERR_OK; /* ����ERR_OK,��ʾ��Ϣ���д����ɹ� ERR_OK=0 */
        else
        {
            myfree(SRAMIN, (*mbox));
            return ERR_MEM;         /* ��Ϣ���д������� */
        }
    }
    else return ERR_MEM;            /* ��Ϣ���д������� */
}

/**
 * @brief       �ͷŲ�ɾ��һ����Ϣ����
 * @param       mbox            : ��Ϣ����
 * @retval      ��
 */
void sys_mbox_free(sys_mbox_t *mbox)
{
    u8_t ucErr;
    sys_mbox_t m_box = *mbox;
    (void)OSQDel(m_box->pQ, OS_DEL_ALWAYS, &ucErr);
    LWIP_ASSERT( "OSQDel ", ucErr == OS_ERR_NONE );
    myfree(SRAMIN, m_box);
    *mbox = NULL;
}

/**
 * @brief       ����Ϣ�����з���һ����Ϣ(���뷢�ͳɹ�)
 * @param       mbox            : ��Ϣ����
 * @param       msg             : Ҫ���͵���Ϣ
*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    if (msg == NULL)msg = (void *)&pvNullPointer;       /* ��msgΪ��ʱ msg����pvNullPointerָ���ֵ */

    while (OSQPost((*mbox)->pQ, msg) != OS_ERR_NONE);   /* ��ѭ���ȴ���Ϣ���ͳɹ� */
}

/**
 * @brief       ������һ����Ϣ���䷢����Ϣ
 *  @note       �˺��������sys_mbox_post����ֻ����һ����Ϣ��
 *              ����ʧ�ܺ󲻻᳢�Եڶ��η���
 * @param       mbox            : ��Ϣ����
 * @param       msg             : Ҫ���͵���Ϣ
 * @retval      ERR_OK,����OK; ERR_MEM,����ʧ��;
 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (msg == NULL)msg = (void *)&pvNullPointer; /* ��msgΪ��ʱ msg����pvNullPointerָ���ֵ */

    if ((OSQPost((*mbox)->pQ, msg)) != OS_ERR_NONE)return ERR_MEM;

    return ERR_OK;
}
 
/**
 * @brief       �ȴ������е���Ϣ
 * @param       mbox            : ��Ϣ����
 * @param       msg             : Ҫ���͵���Ϣ
 * @param       timeout         : ��ʱʱ��
 * @retval      �ȴ���Ϣ���õ�ʱ��/SYS_ARCH_TIMEOUT
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    u8_t ucErr;
    u32_t ucos_timeout, timeout_new;
    void *temp;
    sys_mbox_t m_box = *mbox;

    if (timeout != 0)
    {
        ucos_timeout = (timeout * OS_TICKS_PER_SEC) / 1000; /* ת��Ϊ������,��ΪUCOS��ʱʹ�õ��ǽ�����,��LWIP����ms */

        if (ucos_timeout < 1)ucos_timeout = 1;              /* ����1������ */
    }
    else ucos_timeout = 0;

    timeout = OSTimeGet();                                  /* ��ȡϵͳʱ�� */
    temp = OSQPend(m_box->pQ, (u16_t)ucos_timeout, &ucErr); /* ������Ϣ����,�ȴ�ʱ��Ϊucos_timeout */

    if (msg != NULL)
    {
        if (temp == (void *)&pvNullPointer)*msg = NULL;     /* ��Ϊlwip���Ϳ���Ϣ��ʱ������ʹ����pvNullPointerָ��,�����ж�pvNullPointerָ���ֵ */
        else *msg = temp;                                   /* �Ϳ�֪�����󵽵���Ϣ�Ƿ���Ч */
    }

    if (ucErr == OS_ERR_TIMEOUT)timeout = SYS_ARCH_TIMEOUT; /* ����ʱ */
    else
    {
        LWIP_ASSERT("OSQPend ", ucErr == OS_ERR_NONE);
        timeout_new = OSTimeGet();

        if (timeout_new > timeout) timeout_new = timeout_new - timeout; /* ���������Ϣ��ʹ�õ�ʱ�� */
        else timeout_new = 0xffffffff - timeout + timeout_new;

        timeout = timeout_new * 1000 / OS_TICKS_PER_SEC + 1;
    }

    return timeout;
}

/**
 * @brief       ���Ի�ȡ��Ϣ
 * @param       mbox            : ��Ϣ����
 * @param       msg             : ��Ϣ
 * @retval      �ȴ���Ϣ���õ�ʱ��/SYS_ARCH_TIMEOUT
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    return sys_arch_mbox_fetch(mbox, msg, 1); /* ���Ի�ȡһ����Ϣ */
}

/**
 * @brief       ���һ����Ϣ�����Ƿ���Ч
 * @param       mbox            : ��Ϣ����
 * @retval      1,��Ч; 0,��Ч;
 */
int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (int)(*mbox);
}

/**
 * @brief       ����һ����Ϣ����Ϊ��Ч
 * @param       mbox            : ��Ϣ����
 */
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = NULL;
}

/**
 * @brief       ����һ���ź���
 * @param       sem             : �������ź���
 * @param       count           : �ź���ֵ
 * @retval      ERR_OK,����OK; ERR_MEM,����ʧ��;
 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    u8_t err;
    *sem = OSSemCreate((u16_t)count);

    if (*sem == NULL)return ERR_MEM;

    OSEventNameSet(*sem, "LWIP Sem", &err);
    LWIP_ASSERT("OSSemCreate ", *sem != NULL );
    return ERR_OK;
}

/**
 * @brief       �ȴ�һ���ź���
 * @param       sem             : Ҫ�ȴ����ź���
 * @param       timeout         : ��ʱʱ��
 * @retval      �ȴ��ź������õ�ʱ��/SYS_ARCH_TIMEOUT
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    u8_t ucErr;
    u32_t ucos_timeout, timeout_new;

    if (	timeout != 0)
    {
        ucos_timeout = (timeout * OS_TICKS_PER_SEC) / 1000;    /* ת��Ϊ������,��ΪUCOS��ʱʹ�õ��ǽ�����,��LWIP����ms */

        if (ucos_timeout < 1)
            ucos_timeout = 1;
    }
    else ucos_timeout = 0;

    timeout = OSTimeGet();
    OSSemPend (*sem, (u16_t)ucos_timeout, (u8_t *)&ucErr);

    if (ucErr == OS_ERR_TIMEOUT)timeout = SYS_ARCH_TIMEOUT;    /* ����ʱ */
    else
    {
        timeout_new = OSTimeGet();

        if (timeout_new >= timeout) timeout_new = timeout_new - timeout;
        else timeout_new = 0xffffffff - timeout + timeout_new;

        timeout = (timeout_new * 1000 / OS_TICKS_PER_SEC + 1); /* ���������Ϣ��ʹ�õ�ʱ��(ms) */
    }

    return timeout;
}

/**
 * @brief       ����һ���ź���
 * @param       sem             : �ź���ָ��
 */
void sys_sem_signal(sys_sem_t *sem)
{
    OSSemPost(*sem);
}

/**
 * @brief       �ͷŲ�ɾ��һ���ź���
 * @param       sem             : �ź���ָ��
 */
void sys_sem_free(sys_sem_t *sem)
{
    u8_t ucErr;
    (void)OSSemDel(*sem, OS_DEL_ALWAYS, &ucErr );

    if (ucErr != OS_ERR_NONE)LWIP_ASSERT("OSSemDel ", ucErr == OS_ERR_NONE);

    *sem = NULL;
}

/**
 * @brief       ��ѯһ���ź�����״̬,��Ч����Ч
 * @param       sem             : �ź���ָ�� 
 * @retval      1,��Ч; 0,��Ч;
 */
int sys_sem_valid(sys_sem_t *sem)
{
    OS_SEM_DATA  sem_data;
    return (OSSemQuery (*sem, &sem_data) == OS_ERR_NONE ) ? 1 : 0;
}

/**
 * @brief       ����һ���ź�����Ч
 * @param       sem             : �ź���ָ��
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = NULL;
}

/* arch��ʼ�� */
void sys_init(void)
{
    /* ����,�����ڸú���,�����κ����� */
}

extern OS_STK *TCPIP_THREAD_TASK_STK; /* TCP IP�ں������ջ,��lwip_comm�������� */

/**
 * @brief       ����һ���½���
 * @param       *name           : ��������
 * @param       thred           : ����������
 * @param       *arg            : �����������Ĳ���
 * @param       stacksize       : ��������Ķ�ջ��С
 * @param       prio            : ������������ȼ�
 */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    OS_CPU_SR cpu_sr;

    if (strcmp(name, TCPIP_THREAD_NAME) == 0) /* ����TCP IP�ں����� */
    {
        OS_ENTER_CRITICAL();                  /* �����ٽ��� */
        OSTaskCreate(thread, arg, (OS_STK *)&TCPIP_THREAD_TASK_STK[stacksize - 1], prio); /* ����TCP IP�ں����� */
        OS_EXIT_CRITICAL();                   /* �˳��ٽ��� */
    }

    return 0;
}

/**
 * @param       lwip            : ��ʱ����
 * @param       ms              : Ҫ��ʱ��ms��
 */
void sys_msleep(u32_t ms)
{
    delay_ms(ms);
}

/**
 * @brief       ��ȡϵͳʱ��,LWIP1.4.1���ӵĺ���
 * @retval      ��ǰϵͳʱ��(��λ:����)
 */
u32_t sys_now(void)
{
    u32_t ucos_time, lwip_time;
    ucos_time = OSTimeGet();                                /* ��ȡ��ǰϵͳʱ�� �õ�����UCSO�Ľ����� */
    lwip_time = (ucos_time * 1000 / OS_TICKS_PER_SEC + 1);  /* ��������ת��ΪLWIP��ʱ��MS */
    return lwip_time;                                       /* ����lwip_time; */
}













































