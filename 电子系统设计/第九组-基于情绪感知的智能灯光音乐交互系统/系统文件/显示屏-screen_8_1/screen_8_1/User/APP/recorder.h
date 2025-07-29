/**
 ****************************************************************************************************
 * @file        camera.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-12-1
 * @brief       APP-¼���� ����
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20221201
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */
 
#ifndef __RECORDER_H
#define __RECORDER_H

#include "common.h"
#include "wavplay.h"


#define REC_I2S_RX_DMA_BUF_SIZE     4096        /* ����RX DMA �����С */
#define REC_I2S_RX_FIFO_SIZE        10          /* �������FIFO��С */

/* ��ͼ��/ͼƬ·�� */
extern uint8_t *const RECORDER_DEMO_PIC;        /* demoͼƬ·�� */
extern uint8_t *const RECORDER_RECR_PIC;        /* ¼�� �ɿ� */
extern uint8_t *const RECORDER_RECP_PIC;        /* ¼�� ���� */
extern uint8_t *const RECORDER_PAUSER_PIC;      /* ��ͣ �ɿ� */
extern uint8_t *const RECORDER_PAUSEP_PIC;      /* ��ͣ ���� */
extern uint8_t *const RECORDER_STOPR_PIC;       /* ֹͣ �ɿ� */
extern uint8_t *const RECORDER_STOPP_PIC;       /* ֹͣ ���� */


uint8_t recoder_i2s_fifo_read(uint8_t **buf);
uint8_t recoder_i2s_fifo_write(uint8_t *buf);

void rec_i2s_dma_rx_callback(void) ;
void recorder_enter_rec_mode(void);
void recorder_stop_rec_mode(void);
void recoder_set_samplerate(__WaveHeader *wavhead, uint16_t samplerate);
void recorder_wav_init(__WaveHeader *wavhead);  /* ��ʼ��WAVͷ */
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























