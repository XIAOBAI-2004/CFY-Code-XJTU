/**
 ****************************************************************************************************
 * @file        camera.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-1
 * @brief       APP-录音机 代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20221201
 * 1, 修改注释方式
 * 2, 修改u8/u16/u32为uint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */
 
#ifndef __RECORDER_H
#define __RECORDER_H

#include "common.h"
#include "wavplay.h"


#define REC_I2S_RX_DMA_BUF_SIZE     4096        /* 定义RX DMA 数组大小 */
#define REC_I2S_RX_FIFO_SIZE        10          /* 定义接收FIFO大小 */

/* 各图标/图片路径 */
extern uint8_t *const RECORDER_DEMO_PIC;        /* demo图片路径 */
extern uint8_t *const RECORDER_RECR_PIC;        /* 录音 松开 */
extern uint8_t *const RECORDER_RECP_PIC;        /* 录音 按下 */
extern uint8_t *const RECORDER_PAUSER_PIC;      /* 暂停 松开 */
extern uint8_t *const RECORDER_PAUSEP_PIC;      /* 暂停 按下 */
extern uint8_t *const RECORDER_STOPR_PIC;       /* 停止 松开 */
extern uint8_t *const RECORDER_STOPP_PIC;       /* 停止 按下 */


uint8_t recoder_i2s_fifo_read(uint8_t **buf);
uint8_t recoder_i2s_fifo_write(uint8_t *buf);

void rec_i2s_dma_rx_callback(void) ;
void recorder_enter_rec_mode(void);
void recorder_stop_rec_mode(void);
void recoder_set_samplerate(__WaveHeader *wavhead, uint16_t samplerate);
void recorder_wav_init(__WaveHeader *wavhead);  /* 初始化WAV头 */
uint8_t recorder_vu_get(uint16_t signallevel);
void recorder_vu_meter(uint16_t x, uint16_t y, uint8_t level);
void recorder_show_time(uint16_t x, uint16_t y, uint32_t tsec);
void recorder_show_name(uint16_t x, uint16_t y, uint8_t *name);
void recorder_show_samplerate(uint16_t x, uint16_t y, uint16_t samplerate);
void recorder_load_ui(void);
void recorder_new_pathname(uint8_t *pname);
void recorder_show_agc(uint16_t x, uint16_t y, uint8_t agc);
uint8_t recorder_agc_set(uint16_t x, uint16_t y, uint8_t *agc, uint8_t *caption);
uint8_t recorder_play(void);

#endif























