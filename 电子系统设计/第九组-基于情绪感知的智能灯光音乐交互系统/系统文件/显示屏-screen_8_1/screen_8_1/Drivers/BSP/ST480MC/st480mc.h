/**
 ****************************************************************************************************
 * @file        st480mc.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-03-29
 * @brief       ST480MC 驱动代码
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
 * V1.0 20220329
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __ST480MC_H
#define __ST480MC_H

#include "./SYSTEM/sys/sys.h"


#define ST480MC_ADDR            0x18    /* ST480MC IIC器件地址(A0 = 0时) */
#define ST480MC_RESET           0XF0    /* ST480MC 复位命令 */
#define ST480MC_READ_REG        0x50    /* ST480MC 读寄存器 */
#define ST480MC_WRITE_REG       0x60    /* ST480MC 写寄存器 */
#define ST480MC_READ_DATA       0x4F    /* ST480MC 读取全部数据(zxyt) */
#define ST480MC_BURST_MODE      0x1F    /* ST480MC 突发模式读取数据(zxyt) */
#define ST480MC_SINGLR_MODE     0x3F    /* ST480MC 单次测量读取数据(zxyt) */


#define ST480MC_SENS_XY         667     /* X,Y 轴灵敏度; 单位:  LSB/Gauss */
#define ST480MC_SENS_Z          400     /* Z   轴灵敏度  单位:: LSB/Gauss */


uint8_t st480mc_init(void);                                                             /* 初始化 */
uint8_t st480mc_read_nbytes(uint8_t addr, uint8_t length, uint8_t *buf);                /* 读N字节,可以发送命令 */
uint16_t st480mc_read_register(uint8_t reg);                                            /* 读寄存器 */
uint8_t st480mc_write_register(uint8_t reg, uint16_t data);                             /* 写寄存器 */

uint8_t st480mc_read_magdata(int16_t *pmagx, int16_t *pmagy, int16_t *pmagz);           /* 读取磁力原始数据(单次) */
uint8_t st480mc_read_temperature(float *ptemp);                                         /* 读取温度传感器值(单次,℃) */
uint8_t st480mc_read_magdata_average(int16_t *pmagx, int16_t *pmagy, int16_t *pmagz, uint8_t times);    /* 读取磁力原始数据(平均值) */


#endif













