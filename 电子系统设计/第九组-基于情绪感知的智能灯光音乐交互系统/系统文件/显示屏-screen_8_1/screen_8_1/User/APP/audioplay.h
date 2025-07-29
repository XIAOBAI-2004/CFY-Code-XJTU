/**
 ****************************************************************************************************
 * @file        audioplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-11-25
 * @brief       APP-���ֲ����� ����
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
 * V1.1 20221125
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 *
 ****************************************************************************************************
 */

#ifndef __AUDIOPLAY_H
#define __AUDIOPLAY_H

#include "./SYSTEM/sys/sys.h"
#include "os.h"
#include "common.h"
#include "gui.h"
#include "lyric.h"

#include "ucos_ii.h"

/* ��ͼ��/ͼƬ·�� */
extern uint8_t *const AUDIO_BTN_PIC_TBL[2][5];  /* 5��ͼƬ��ť��·�� */
extern uint8_t *const AUDIO_BACK_PIC[3];        /* ��������ͼƬ */
extern uint8_t *const AUDIO_PLAYR_PIC;          /* ���� �ɿ� */
extern uint8_t *const AUDIO_PLAYP_PIC;          /* ���� ���� */
extern uint8_t *const AUDIO_PAUSER_PIC;         /* ��ͣ �ɿ� */
extern uint8_t *const AUDIO_PAUSEP_PIC;         /* ��ͣ ���� */

/* ����ɫ���� */
#define AUDIO_TITLE_COLOR       0XFFFF          /* ������������ɫ */
#define AUDIO_TITLE_BKCOLOR     0X0000          /* ���������ⱳ��ɫ */

#define AUDIO_INFO_COLOR        0X8410          /* ��Ϣ�������ɫ */
#define AUDIO_MAIN_BKCOLOR      0X18E3          /* ������ɫ */
#define AUDIO_BTN_BKCOLOR       0XDF9F          /* 5�����ư�ť����ɫ */

#define AUDIO_LRC_MCOLOR        0XF810          /* ��ǰ�����ɫΪ��ɫ */
#define AUDIO_LRC_SCOLOR        0XFFFF          /* ǰһ��ͺ�һ������ɫΪ��ɫ */


/* ���ֲ��Ų���������� */
typedef enum
{
    AP_OK = 0X00,       /* ����������� */
    AP_NEXT,            /* ������һ�� */
    AP_PREV,            /* ������һ�� */
    AP_ERR = 0X80,      /* �����д���(û����������,������ʾ����) */
} APRESULT;

/* ���ֲ��ſ����� */
typedef __packed struct
{
    /* 2��I2S�����BUF */
    uint8_t *i2sbuf1;
    uint8_t *i2sbuf2;
    uint8_t *tbuf;                  /* ��ʱ���� */
    
    FIL *file;                      /* ��Ƶ�ļ�ָ�� */
    uint32_t(*file_seek)(uint32_t); /* �ļ�������˺��� */

    volatile uint8_t status;        /* bit0:0,��ͣ����;1,�������� */
                                    /* bit1:0,��������;1,�������� */
                                    /* bit2~3:���� */
                                    /* bit4:0,�����ֲ���;1,���ֲ����� (������) */
                                    /* bit5:0,�޶���;1,ִ����һ���и����(������) */
                                    /* bit6:0,�޶���;1,������ֹ����(���ǲ�ɾ����Ƶ��������),������ɺ�,���������Զ������λ */
                                    /* bit7:0,��Ƶ����������ɾ��/����ɾ��;1,��Ƶ����������������(�������ִ��) */

    uint8_t mode;           /* ����ģʽ */
                            /* 0,ȫ��ѭ��;1,����ѭ��;2,�������; */

    uint8_t *path;          /* ��ǰ�ļ���·�� */
    uint8_t *name;          /* ��ǰ���ŵ�MP3�������� */
    uint16_t namelen;       /* name��ռ�ĵ��� */
    uint16_t curnamepos;    /* ��ǰ��ƫ�� */

    uint32_t totsec ;       /* ���׸�ʱ��,��λ:�� */
    uint32_t cursec ;       /* ��ǰ����ʱ�� */
    uint32_t bitrate;       /* ������(λ��) */
    uint32_t samplerate;        /* ������ */
    uint16_t bps;           /* λ��,����16bit,24bit,32bit */

    uint16_t curindex;      /* ��ǰ���ŵ���Ƶ�ļ����� */
    uint16_t mfilenum;      /* �����ļ���Ŀ */
    uint16_t *mfindextbl;   /* ��Ƶ�ļ������� */

} __audiodev;
extern __audiodev g_audiodev; /* ���ֲ��ſ����� */

/* ���ֲ��Ž���,UIλ�ýṹ�� */
typedef __packed struct
{
    /* ����������ز��� */
    uint8_t tpbar_height;   /* �����������߶� */
    uint8_t capfsize;       /* ���������С */
    uint8_t msgfsize;       /* ��ʾ��Ϣ�����С(������/����/������/������/λ��/���ʵ�/����ʱ����) */
    uint8_t lrcdheight;     /* ����м�� */

    /* �м���Ϣ����ز��� */
    uint8_t msgbar_height;  /* ��Ϣ���߶� */
    uint8_t nfsize;         /* �����������С */
    uint8_t xygap;          /* x,y�����ƫ����,������/����ͼ��/λ����� 1 gap,������Ϣ,1/2 gap */
    uint16_t vbarx;         /* ������x���� */
    uint16_t vbary;         /* ������y���� */
    uint16_t vbarwidth;     /* ���������� */
    uint16_t vbarheight;    /* ��������� */
    uint8_t msgdis;         /* ������(������) dis+����ͼ��+vbar+dis+������+λ��+dis */

    /* ���Ž�����ز��� */
    uint8_t prgbar_height;  /* �������߶� */
    uint16_t pbarwidth;     /* ���������� */
    uint16_t pbarheight;    /* ��������� */
    uint16_t pbarx;         /* ������x���� */
    uint16_t pbary;         /* ������y���� */

    /* ��ť����ز��� */
    uint8_t btnbar_height;  /* ��ť���߶� */
} __audioui;
extern __audioui *g_aui;      /* ���ֲ��Ž�������� */


/* ȡ2��ֵ����Ľ�Сֵ */
#ifndef AUDIO_MIN
#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))
#endif

void audio_start(void);
void audio_stop(void);

void music_play_task(void *pdata);
void audio_stop_req(__audiodev *audiodevx);
uint8_t audio_filelist(__audiodev *audiodevx);
void audio_load_ui(uint8_t mode);
void audio_show_vol(uint8_t pctx);
void audio_time_show(uint16_t sx, uint16_t sy, uint16_t sec);
void audio_info_upd(__audiodev *audiodevx, _progressbar_obj *audioprgbx, _progressbar_obj *volprgbx, _lyric_obj *lrcx);
void audio_lrc_bkcolor_process(_lyric_obj *lrcx, uint8_t mode);
void audio_lrc_show(__audiodev *audiodevx, _lyric_obj *lrcx);
uint8_t audio_task_creat(void);
void audio_task_delete(void);
uint8_t audio_play(void);

/* ��audioplay.h����״̬���� */
#define PLAYER_STOPPED  0
#define PLAYER_PLAYING  1
#define PLAYER_PAUSED   2
//extern uint8_t g_player_state;


/* ��audioplay.h��ͷ����������� */
/* ��voice_cmd_tö���������������� */
typedef enum {
    VOICE_CMD_NONE,
    VOICE_CMD_PLAY,     
    VOICE_CMD_PAUSE,    
    VOICE_CMD_PREV,     
    VOICE_CMD_NEXT,
//	 VOICE_CMD_LIUSHUI,    // ��ˮ
//	VOICE_CMD_QISEJIANBIAN,     // ��ɫ����
//VOICE_CMD_SHANSHUO,    //��˸
//	VOICE_CMD_XUNHUAN,      //  ѭ��
//VOICE_CMD_KAIDENG,   // ����
//	VOICE_CMD_GUANDENG,     // �ص�
//	VOICE_CMD_KAIXIN,     // ����
//	VOICE_CMD_FENNU,    // ��ŭ
//	VOICE_CMD_BEISHANG,     // ����
//	VOICE_CMD_KUAILE,     // ����
//VOICE_CMD_FUZA,     // ����
    VOICE_CMD_PLAY_SONG /* ��������ָ���������� */
} voice_cmd_t;

#define MAX_SONG_NAME_LEN 64  // UTF-8������ÿ����ռ3�ֽ�

/* ������������������ṹ�� */
// ���ļ�ͷ�����
#pragma pack(1)
typedef struct {
    voice_cmd_t cmd_type;
    uint8_t song_name[MAX_SONG_NAME_LEN]; // UTF-8����
} voice_cmd_ex_t;
#pragma pack()




// �޸Ĳ��Һ���ԭ��
int16_t audio_find_song_index_utf8(const uint8_t* song_name_utf8);
int16_t audio_find_song_index_utf88(const uint8_t* song_name_utf8);

/* ��audioplay.h����� */
int utf8_strcmp(const char *s1, const char *s2);

/* ����ת����� */
#define UTF8_TO_GBK(utf8, gbk, gbk_size)   utf8_to_gbk(utf8, gbk, gbk_size) 
int utf8_to_gbk(const uint8_t *utf8, uint8_t *gbk, int gbk_buf_size);

/* ����ȫ����Ϣ���� */
extern OS_EVENT *g_voice_q;

/* �������ƺ��� */
void audio_control_prev_next(uint8_t dir);
void audio_ui_refresh(_btn_obj **btns);
void btn_state_update(_btn_obj *btn, const uint8_t *upath, const uint8_t *dpath);

#define VOICE_STK_SIZE 256  // ������Ҫ������ջ��С


#define VOICE_STK_SIZE 256  // ������Ҫ������ջ��С

#ifndef OS_EVENT_WAIT_OPTION_NONE
#define OS_EVENT_WAIT_OPTION_NONE 0
#endif

#ifndef OS_Q_EMPTY
#define OS_Q_EMPTY ((void *)0)
#endif

void init_voice_control(void);

extern _btn_obj *g_tbtn[5];  // ȫ�ְ�ť��������

// audioplay.h
/* ��audioplay.h������������� */
extern _filelistbox_obj *g_active_flistbox; // ��ǰ����ļ��б�ʵ��
extern uint8_t g_last_valid_path[256];      // �����Ч·������

/* filelistbox.h */
void filelistbox_2click_check(_filelistbox_obj *filelistbox);

//extern volatile uint8_t g_player_state; // PLAYER_PLAYING/PAUSED/STOPPED

#endif








