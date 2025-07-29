/**
 ****************************************************************************************************
 * @file        stmflash.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       STM32内部FLASH读写 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20220111
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/STMFLASH/stmflash.h"


/**
 * @brief       解锁STM32的FLASH
 * @param       无
 * @retval      无
 */
static void stmflash_unlock(void)
{
    FLASH->KEYR = STM32_FLASH_KEY1;     /* FLASH 写入解锁序列. */
    FLASH->KEYR = STM32_FLASH_KEY2;
}

/**
 * @brief       flash上锁
 * @param       无
 * @retval      无
 */
static void stmflash_lock(void)
{
    FLASH->CR |= (uint32_t)1 << 31;     /* FLASH 上锁 */
}

/**
 * @brief       得到FLASH的错误状态
 * @param       无
 * @retval      执行结果
 *   @arg       0   : 已完成
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_get_error_status(void)
{
    uint32_t res = 0;
    res = FLASH->SR;

    if (res & (1 << 16)) return 1;  /* BSY=1, 繁忙 */
    if (res & (1 << 7))  return 2;  /* PGSERR=1,编程序列错误 */
    if (res & (1 << 6))  return 3;  /* PGPERR=1,编程并行位数错误 */
    if (res & (1 << 5))  return 4;  /* PGAERR=1,编程对齐错误 */
    if (res & (1 << 4))  return 5;  /* WRPERR=1,写保护错误 */

    return 0;   /* 没有任何状态/操作完成. */
}

/**
 * @brief       等待操作完成
 * @param       time : 要延时的长短
 * @retval      执行结果
 *   @arg       0   : 已完成
 *   @arg       0XFF: 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_wait_done(uint32_t time)
{
    uint8_t res;

    do
    {
        res = stmflash_get_error_status();

        if (res != 1)
        {
            break;      /* 非忙, 无需等待了, 直接退出 */
        }
        
        time--;
    } while (time);

    if (time == 0)res = 0XFF;   /* 超时 */

    return res;
}

/**
 * @brief       擦除扇区
 * @param       saddr   : 扇区地址 0 ~ 11
 *                        0~3, 16K扇区; 4,64K扇区; 5~11, 128K扇区.
 * @retval      执行结果
 *   @arg       0   : 已完成
 *   @arg       0XFF: 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_erase_sector(uint32_t saddr)
{
    uint8_t res = 0;

    res = stmflash_wait_done(0XFFFFFFFF);   /* 等待上次操作结束 */

    if (res == 0)
    {
        FLASH->CR &= ~(3 << 8);             /* 清除PSIZE原来的设置 */
        FLASH->CR |= 2 << 8;                /* 设置为32bit宽,确保VCC=2.7~3.6V之间!! */
        FLASH->CR &= ~(0X1F << 3);          /* 清除原来的设置 */
        FLASH->CR |= saddr << 3;            /* 设置要擦除的扇区 */
        FLASH->CR |= 1 << 1;                /* 扇区擦除 */
        FLASH->CR |= 1 << 16;               /* 开始擦除 */
        res = stmflash_wait_done(0XFFFFFFFF);   /* 等待操作结束 */

        if (res != 1)                       /* 非忙 */
        {
            FLASH->CR &= ~(1 << 1);         /* 清除扇区擦除标志 */
        }
    }

    return res;
}

/**
 * @brief       在FLASH指定地址写一个字 (32位数据)
 *   @note      这了写入一个字, 是指4个字节
 * @param       faddr   : 写入地址 (此地址必须为4的倍数!!)
 * @param       data    : 要写入的数据(32位)
 * @retval      执行结果
 *   @arg       0   : 已完成
 *   @arg       0XFF: 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_write_word(uint32_t faddr, uint32_t data)
{
    uint8_t res;

    res = stmflash_wait_done(0XFFFFF);

    if (res == 0)   /* OK */
    {
        FLASH->CR &= ~(3 << 8);             /* 清除PSIZE原来的设置 */
        FLASH->CR |= 2 << 8;                /* 设置为32bit宽,确保VCC=2.7~3.6V之间!! */
        FLASH->CR |= 1 << 0;                /* 编程使能 */
        *(volatile uint32_t *)faddr = data; /* 写入数据 */
        res = stmflash_wait_done(0XFFFFF);  /* 等待操作完成,一个字编程 */

        if (res != 1)                       /* 操作成功 */
        {
            FLASH->CR &= ~(1 << 0);         /* 清除PG位 */
        }
    }

    return res;
}

/**
 * @brief       从指定地址读取一个字 (32位数据)
 * @param       faddr   : 读取地址 (此地址必须为4的倍数!!)
 * @retval      读取到的数据 (32位)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       获取某个地址所在的flash扇区
 * @param       faddr   : flash地址
 * @retval      0~11, 即addr所在的扇区
 */
static uint8_t stmflash_get_flash_sector(uint32_t addr)
{
    if (addr < ADDR_FLASH_SECTOR_1)return 0;
    else if (addr < ADDR_FLASH_SECTOR_2)return 1;
    else if (addr < ADDR_FLASH_SECTOR_3)return 2;
    else if (addr < ADDR_FLASH_SECTOR_4)return 3;
    else if (addr < ADDR_FLASH_SECTOR_5)return 4;
    else if (addr < ADDR_FLASH_SECTOR_6)return 5;
    else if (addr < ADDR_FLASH_SECTOR_7)return 6;
    else if (addr < ADDR_FLASH_SECTOR_8)return 7;
    else if (addr < ADDR_FLASH_SECTOR_9)return 8;
    else if (addr < ADDR_FLASH_SECTOR_10)return 9;
    else if (addr < ADDR_FLASH_SECTOR_11)return 10;

    return 11;
}

/**
 * @brief       在FLASH 指定位置, 写入指定长度的数据(自动擦除)
 *   @note      因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数写地址如果非0XFF
 *              ,那么会先擦除整个扇区且不保存扇区数据.所以写非0XFF的地址,将导致整个扇区数据丢失.
 *              建议写之前确保扇区里没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
 *              该函数对OTP区域也有效!可以用来写OTP区!
 *              OTP区域地址范围:0X1FFF7800~0X1FFF7A0F(注意：最后16字节，用于OTP数据块锁定，别乱写！！)
 * @param       waddr   : 起始地址 (此地址必须为4的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 字(32位)数(就是要写入的32位数据的个数)
 * @retval      无
 */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{

    uint8_t status = 0;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 4 ||        /* 写入地址小于 STM32_FLASH_BASE, 或不是4的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE))  /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }
    
    stmflash_unlock();              /* FLASH解锁 */
    
    FLASH->ACR &= ~(1 << 10);       /* FLASH擦除期间,必须禁止数据缓存!!!搞了我两晚上才发现这个问题! */
    addrx = waddr;                  /* 写入的起始地址 */
    endaddr = waddr + length * 4;   /* 写入的结束地址 */

    if (addrx < 0X1FFF0000)         /* 只有主存储区,才需要执行擦除操作!! */
    {
        while (addrx < endaddr)     /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF)    /* 有非0XFFFFFFFF的地方,要擦除这个扇区 */
            {
                status = stmflash_erase_sector(stmflash_get_flash_sector(addrx));

                if (status)break;   /* 发生错误了 */
            }
            else
            {
                addrx += 4;
            }
        }
    }

    if (status == 0)
    {
        while (waddr < endaddr)     /* 写数据 */
        {
            if (stmflash_write_word(waddr, *pbuf))  /* 写入数据 */
            {
                break;              /* 写入异常 */
            }

            waddr += 4;
            pbuf++;
        }
    }

    FLASH->ACR |= 1 << 10;          /* FLASH擦除结束,开启数据fetch */
    
    stmflash_lock();                /* 上锁 */
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的 字(32位)数(就是要读取的32位数据的个数)
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr);    /* 读取4个字节(1个字) */
        raddr += 4;                             /* 偏移4个字节 */
    }
}

/******************************************************************************************/
/* 测试用代码 */

/**
 * @brief       测试写数据(写1个字)
 * @param       waddr : 起始地址
 * @param       wdata : 要写入的数据
 * @retval      无
 */
void test_write(uint32_t waddr, uint32_t wdata)
{
    stmflash_write(waddr, &wdata, 1);       /* 写入一个字 */
}


















