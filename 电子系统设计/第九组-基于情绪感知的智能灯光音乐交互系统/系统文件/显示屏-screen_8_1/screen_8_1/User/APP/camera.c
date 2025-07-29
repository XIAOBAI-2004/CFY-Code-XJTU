/**
 ****************************************************************************************************
 * @file        camera.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-11-30
 * @brief       APP-����� ����
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
 * V1.0 20221130
 * 1, �޸�ע�ͷ�ʽ
 * 2, �޸�u8/u16/u32Ϊuint8_t/uint16_t/uint32_t
 ****************************************************************************************************
 */

#include "camera.h"
#include "common.h"
#include "calendar.h"
#include "audioplay.h"
#include "./BSP/OV2640/ov2640.h"
#include "./BSP/DCMI/dcmi.h"


volatile uint8_t hsync_int = 0;         /* ֡�жϱ�־ */
volatile uint8_t jpeg_size;             /* jpegͼƬ�ֱ��� */
volatile uint8_t ov2640_mode = OV2640_RGB565_MODE;  /* ����ģʽ:0,RGB565ģʽ;1,JPEGģʽ */

#define jpeg_dma_bufsize    5*1024      /* ����JPEG DMA����ʱ���ݻ���jpeg_buf0/1�Ĵ�С(*4�ֽ�) */
volatile uint32_t jpeg_data_len = 0;    /* buf�е�JPEG��Ч���ݳ���(*4�ֽ�) */
volatile uint8_t jpeg_data_ok = 0;      /* JPEG���ݲɼ���ɱ�־
                                         * 0,����û�вɼ���;
                                         * 1,���ݲɼ�����,���ǻ�û����;
                                         * 2,�����Ѿ����������,���Կ�ʼ��һ֡����
                                         */

uint32_t *jpeg_buf0;                    /* JPEG���ݻ���buf,ͨ��malloc�����ڴ� */
uint32_t *jpeg_buf1;                    /* JPEG���ݻ���buf,ͨ��malloc�����ڴ� */
uint32_t *jpeg_data_buf;                /* JPEG���ݻ���buf,ͨ��malloc�����ڴ� */


/* JPEG�ߴ�֧���б� */
const uint16_t camera_jpeg_img_size_tbl[][2] =
{
    160, 120,   /* QQVGA */
    176, 144,   /* QCIF */
    320, 240,   /* QVGA */
    400, 240,   /* WQVGA */
    352, 288,   /* CIF */
    640, 480,   /* VGA */
    800, 600,   /* SVGA */
    1024, 768,  /* XGA */
    1280, 800,  /* WXGA */
    1280, 960,  /* XVGA */
    1440, 900,  /* WXGA+ */
    1280, 1024, /* SXGA */
    1600, 1200, /* UXGA */
};
    
/* ����ͷ��ʾ */
uint8_t *const camera_remind_tbl[4][GUI_LANGUAGE_NUM] =
{
    {"��ʼ��OV2640,���Ժ�...", "��ʼ��OV2640,Ո����...", "OV2640 Init,Please wait...",},
    {"δ��⵽OV2640,����...", "δ�z�y��OV2640,Ո�z��...", "No OV2640 find,Please check...",},
    {"����Ϊ:", "�����:", "SAVE AS:",},
    {"�ֱ���", "�ֱ���", "Resolution",},
};

/* �s����ʾ */
uint8_t *const camera_scalemsg_tbl[2][GUI_LANGUAGE_NUM] =
{
    "1:1��ʾ(������)��", "1:1�@ʾ(�o�s��)��", "1:1 Display(No Scale)��",
    "ȫ�ߴ����ţ�", "ȫ�ߴ�s�ţ�", "Full Scale��",
};

/* JPEGͼƬ 13�ֳߴ� */
uint8_t *const camera_jpegimgsize_tbl[13] =
{"QQVGA", "QCIF", "QVGA", "WQVGA", "CIF", "VGA", "SVGA", "XGA", "WXGA", "XVGA", "WXGA+", "SXGA", "UXGA"};

/* ���ճɹ���ʾ����� */
uint8_t *const camera_saveok_caption[GUI_LANGUAGE_NUM] =
{
    "���ճɹ���", "���ճɹ���", "Take Photo OK��",
};

/* ����ʧ����ʾ��Ϣ */
uint8_t *const camera_failmsg_tbl[3][GUI_LANGUAGE_NUM] =
{
    "�����ļ�ʧ��,����!", "�����ļ�ʧ��,Ո�z��!", "Creat File Failed,Please check!",
    "�ڴ治��!", "�ȴ治��!", "Out of memory!",
    "���ݴ���(ͼƬ�ߴ�̫��)!", "�����e�`(�DƬ�ߴ�̫��)!", "Data Error(Picture is too big)!",
};

/* 00������ѡ������ */
uint8_t *const camera_l00fun_caption[GUI_LANGUAGE_NUM] =
{
    "�������", "���C�O��", "Camera Set",
};

/* 00������ѡ��� */
uint8_t *const camera_l00fun_table[GUI_LANGUAGE_NUM][6] =
{
    {"��������", "��Ч����", "�ع�ȼ�", "��������", "ɫ������", "�Աȶ�����",},
    {"�����O��", "��Ч�O��", "�ع�ȼ�", "�����O��", "ɫ���O��", "���ȶ��O��",},
    {"Scene", "Effects", "Exposure", "Brightness", "Saturation", "Contrast"},
};

/* 10������ѡ��� */
/* ����ģʽ */
uint8_t *const camera_l10fun_table[GUI_LANGUAGE_NUM][5] =
{
    {"�Զ�", "����", "����", "�칫��", "��ͥ",},
    {"�Ԅ�", "����", "���", "�k����", "��ͥ",},
    {"Auto", "Sunny", "Cloudy", "Office", "Home"},
};

/* 11������ѡ��� */
/* ��Ч���� */
uint8_t *const camera_l11fun_table[GUI_LANGUAGE_NUM][7] =
{
    {"��ͨ", "��Ƭ", "�ڰ�", "ƫ��ɫ", "ƫ��ɫ", "ƫ��ɫ", "����"},
    {"��ͨ", "ؓƬ", "�ڰ�", "ƫ�tɫ", "ƫ�Gɫ", "ƫ�{ɫ", "�͹�"},
    {"Normal", "Negative", "B&W", "Redish", "Greenish", "Bluish", "Antique",},
};

/* 12~14������ѡ��� */
/* ����-2~2��5��ֵ */
uint8_t *const camera_l125fun_table[GUI_LANGUAGE_NUM][5] =
{
    {"-2", "-1", "0", "+1", "+2",},
    {"-2", "-1", "0", "+1", "+2",},
    {"-2", "-1", "0", "+1", "+2",},
};

/**
 * @brief       ����JPEG����
 *  @note       ���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
 * @param       ��
 * @retval      ��
 */
void jpeg_data_process(void)
{
    uint16_t i;
    uint16_t rlen;/* ʣ�����ݳ��� */
    uint32_t *pbuf;

    if (ov2640_mode == OV2640_JPEG_MODE)    /* ֻ����JPEG��ʽ��,����Ҫ������ */
    {
        if (jpeg_data_ok == 0)              /* jpeg���ݻ�δ�ɼ���? */
        {
            DMA2_Stream1->CR &= ~(1 << 0);  /* ֹͣ��ǰ���� */

            while (DMA2_Stream1->CR & 0X01);                /* �ȴ�DMA2_Stream1������ */

            rlen = jpeg_dma_bufsize - DMA2_Stream1->NDTR;   /* �õ�ʣ�����ݳ��� */
            pbuf = jpeg_data_buf + jpeg_data_len;           /* ƫ�Ƶ���Ч����ĩβ,������� */

            if (DMA2_Stream1->CR & (1 << 19))for (i = 0; i < rlen; i++)pbuf[i] = jpeg_buf1[i]; /* ��ȡbuf1�����ʣ������ */
            else for (i = 0; i < rlen; i++)pbuf[i] = jpeg_buf0[i]; /* ��ȡbuf0�����ʣ������ */

            jpeg_data_len += rlen;          /* ����ʣ�೤�� */
            jpeg_data_ok = 1;               /* ���JPEG���ݲɼ��갴��,�ȴ������������� */
        }

        if (jpeg_data_ok == 2)              /* ��һ�ε�jpeg�����Ѿ��������� */
        {
            DMA2_Stream1->NDTR = jpeg_dma_bufsize;  /* ���䳤��Ϊjpeg_buf_size*4�ֽ� */
            DMA2_Stream1->CR |= 1 << 0;     /* ���´��� */
            jpeg_data_ok = 0;               /* �������δ�ɼ� */
            jpeg_data_len = 0;              /* �������¿�ʼ */
        }
    }
    else
    {
        lcd_set_cursor(0, 0);
        lcd_write_ram_prepare();            /* ��ʼд��GRAM */
        hsync_int = 1;
    }
}

/**
 * @brief       jpeg���ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    uint32_t *pbuf;
    pbuf = jpeg_data_buf + jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ */

    if (DMA2_Stream1->CR & (1 << 19))       /* buf0����,��������buf1 */
    {
        for (i = 0; i < jpeg_dma_bufsize; i++)pbuf[i] = jpeg_buf0[i];   /* ��ȡbuf0��������� */

        jpeg_data_len += jpeg_dma_bufsize;  /* ƫ�� */
    }
    else  /* buf1����,��������buf0 */
    {
        for (i = 0; i < jpeg_dma_bufsize; i++)pbuf[i] = jpeg_buf1[i];   /* ��ȡbuf1��������� */

        jpeg_data_len += jpeg_dma_bufsize;  /* ƫ�� */
    }
}

/**
 * @brief       �л�ΪOV2640ģʽ
 *   @note      �л�PC8/PC9/PC11ΪDCMI���ù���(AF13)
 * @param       ��
 * @retval      ��
 */
void sw_ov2640_mode(void)
{
    OV2640_PWDN(0); /* OV2640 Power Up */
    
    /* GPIOC8/9/11�л�Ϊ DCMI�ӿ� */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8 , AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9 , AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11, AF13  DCMI_D4 */
}

/**
 * @brief       �л�ΪSD��ģʽ
 *   @note      �л�PC8/PC9/PC11ΪSDMMC���ù���(AF12)
 * @param       ��
 * @retval      ��
 */
void sw_sdcard_mode(void)
{
    OV2640_PWDN(1); /* OV2640 Power Down */
    
    /* GPIOC8/9/11�л�Ϊ SDIO�ӿ� */ 
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8 , AF12  SD1_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9 , AF12  SD1_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11, AF12  SD1_D3 */
}

/**
 * @brief       �õ��ļ���,��������ʱ������
 *  @note       bmp��ϳ�:����"0:PHOTO/PIC20120321210633.bmp"/"2:PHOTO/PIC20120321210633.bmp"���ļ���
 *              jpg��ϳ�:����"0:PHOTO/PIC20120321210633.jpg"/"2:PHOTO/PIC20120321210633.jpg"���ļ���
 * @param       pname           : ��·��������
 * @param       mode            : 0,����.bmp�ļ�;    1,����.jpg�ļ�;
 * @retval      ��
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    calendar_get_time(&calendar);
    calendar_get_date(&calendar);

    if (mode == 0)
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);         /* ��ѡ������SD�� */
        else if (gui_phy.memdevflag & (1 << 2))sprintf((char *)pname, "2:PHOTO/PIC%04d%02d%02d%02d%02d%02d.bmp", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);    /* SD��������,�򱣴���U�� */
    }
    else
    {
        if (gui_phy.memdevflag & (1 << 0))sprintf((char *)pname, "0:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);         /* ��ѡ������SD�� */
        else if (gui_phy.memdevflag & (1 << 2))sprintf((char *)pname, "2:PHOTO/PIC%04d%02d%02d%02d%02d%02d.jpg", calendar.year, calendar.month, calendar.date, calendar.hour, calendar.min, calendar.sec);    /* SD��������,�򱣴���U�� */
    }
}

/**
 * @brief       OV2640����jpgͼƬ
 * @param       pname           : ��·��������
 * @retval      0, �ɹ�
 *              ����, ʧ��
 */
uint8_t ov2640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res;
    uint32_t bwr;
    uint16_t i;
    uint8_t *pbuf;

    uint16_t datasize = 0;          /* ����д�������� */
    uint32_t datalen = 0;           /* ��д�������� */
    uint8_t  *databuf;              /* ���ݻ��棬����ֱ��д�ⲿSRAM���ݵ�SD��������д��������� */
    
    f_jpg = (FIL *)gui_memin_malloc(sizeof(FIL));       /* ����FIL�ֽڵ��ڴ����� */
    databuf = gui_memin_malloc(4096);                   /* ����4K ѭ��д�ݴ��ڴ� */
    
    jpeg_buf0 = gui_memin_malloc(jpeg_dma_bufsize * 4); /* Ϊjpeg dma���������ڴ� */
    jpeg_buf1 = gui_memin_malloc(jpeg_dma_bufsize * 4); /* Ϊjpeg dma���������ڴ� */
    jpeg_data_buf = gui_memex_malloc(210 * 1024);       /* Ϊjpeg�ļ������ڴ�(���210KB) */
    
    if ( !jpeg_buf1 || !jpeg_data_buf) /* �ڴ�����ʧ�� */
    {
        gui_memin_free(f_jpg);
        gui_memin_free(databuf);
        gui_memin_free(jpeg_buf0);
        gui_memin_free(jpeg_buf1);
        gui_memex_free(jpeg_data_buf);
        return 0XFF;
    }

    ov2640_mode = OV2640_JPEG_MODE; /* ����ΪJPEGģʽ */
    sw_ov2640_mode();       /* �л�ΪOV2640ģʽ */
    dcmi_rx_callback = jpeg_dcmi_rx_callback; /* dcmi dma���ջص����� */
    dcmi_dma_init((uint32_t)jpeg_buf0, (uint32_t)jpeg_buf1, jpeg_dma_bufsize, 2, 1);; /* DCMI DMA����(˫����ģʽ) */
    ov2640_jpeg_mode();     /* �л�ΪJPEGģʽ */
    ov2640_image_win_set(0, 0, 1600, 1200);
    ov2640_outsize_set(camera_jpeg_img_size_tbl[jpeg_size][0], camera_jpeg_img_size_tbl[jpeg_size][1]); /* ��������ߴ� */
    dcmi_start();               /* �������� */

    while (jpeg_data_ok != 1);  /* �ȴ���һ֡ͼƬ�ɼ��� */

    jpeg_data_ok = 2;           /* ���Ա�֡ͼƬ,������һ֡�ɼ� */

    while (jpeg_data_ok != 1);  /* �ȴ��ڶ�֡ͼƬ�ɼ��� */

    jpeg_data_ok = 2;           /* ���Ա�֡ͼƬ,������һ֡�ɼ� */

    while (jpeg_data_ok != 1);  /* �ȴ�����֡ͼƬ�ɼ���,����֡,�ű��浽SD��ȥ */

    dcmi_stop();                /* ֹͣDMA���� */
    ov2640_mode = OV2640_RGB565_MODE; /* �л�ΪRGB565ģʽ */
    sw_sdcard_mode();           /* �л�ΪSD��ģʽ */
    res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ� */

    if (res == 0)
    {
        printf("jpeg data size:%d\r\n", jpeg_data_len * 4); /* ���ڴ�ӡJPEG�ļ���С */
        pbuf = (uint8_t *)jpeg_data_buf;

        for (i = 0; i < jpeg_data_len * 4; i++) /* ����0XFF,0XD8 */
        {
            if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8))break;
        }

        if (i == jpeg_data_len * 4)res = 0XFD; /* û�ҵ�0XFF,0XD8 */
        else    /* �ҵ��� */
        {
            jpeg_data_len = jpeg_data_len * 4 - i;  /* ����JPEGʵ���ֽ�����С */
            pbuf += i;                              /* ƫ�Ƶ�0XFF,0XD8�� */
            
            while(datalen < jpeg_data_len)          /* ѭ��д�룡����ֱ��д�ⲿSRAM���ݵ�SDIO�������������FIFO������� */
            {
                if((jpeg_data_len - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpeg_data_len - datalen; /* �������� */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr);  /* д������ */
                pbuf += datasize;
                jpeg_data_len -= datasize;

                if (res)break;
            }
        }
    }

    jpeg_data_len = 0;
    f_close(f_jpg);
    sw_ov2640_mode();       /* �л�ΪOV2640ģʽ */
    ov2640_rgb565_mode();   /* RGB565ģʽ */
    dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA���� */
    gui_memin_free(f_jpg);
    gui_memin_free(databuf);
    gui_memin_free(jpeg_buf0);
    gui_memin_free(jpeg_buf1);
    gui_memex_free(jpeg_data_buf);
    return res;
}

/**
 * @brief       ����ͷ����
 *  @note       ������Ƭ�ļ�,��������SD��PHOTO�ļ�����.
 * @param       ��
 * @retval      δ�õ�
 */
uint8_t camera_play(void)
{
    uint8_t rval = 0;
    uint8_t res;
    uint8_t *caption = 0;
    uint8_t *pname;
    uint8_t selx = 0;
    uint8_t l00sel = 0, l10sel = 0, l11sel = 0; /* Ĭ��ѡ���� */
    uint8_t l2345sel[4];
    uint8_t *psn;
    uint8_t key;
    uint8_t scale = 1;      /* Ĭ����ȫ�ߴ����� */
    uint8_t tcnt = 0;
    uint16_t tpad_temp_val = g_tpad_default_val;

    if (g_audiodev.status & (1 << 7))   /* ��ǰ�ڷŸ�??����ֹͣ */
    {
        audio_stop_req(&g_audiodev);    /* ֹͣ��Ƶ���� */
        audio_task_delete();            /* ɾ�����ֲ������� */
    }

    /* ��ʾ��ʼ���OV2640 */
    window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);

    if (gui_phy.memdevflag & (1 << 0))f_mkdir("0:PHOTO"); /* ǿ�ƴ����ļ���,��������� */

    if (gui_phy.memdevflag & (1 << 2))f_mkdir("2:PHOTO"); /* ǿ�ƴ����ļ���,��������� */

    if (ov2640_init()) /* ��ʼ��ov2640 */
    {
        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_remind_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);
        delay_ms(500);
        rval = 1;
    }

    pname = gui_memin_malloc(40);   /* ����40���ֽ��ڴ�,���ڴ���ļ��� */
    psn = gui_memin_malloc(50);     /* ����50���ֽ��ڴ�,���ڴ�����ƣ�������Ϊ:0:PHOTO/PIC20120321210633.bmp"������ʾ�� */

    if (!pname || !psn)rval = 1;    /* ����ʧ�� */

    if (rval == 0) /* OV2640���� */
    {

        l2345sel[0] = 2;    /* �ع�����2,ʵ��ֵ0 */
        l2345sel[1] = 2;    /* ����Ĭ��Ϊ2,ʵ��ֵ0 */
        l2345sel[2] = 2;    /* ɫ��Ĭ��Ϊ2,ʵ��ֵ0 */
        l2345sel[3] = 2;    /* �Աȶ�Ĭ��Ϊ2,ʵ��ֵ0 */

        ov2640_auto_exposure(l2345sel[0]);      /* �Զ��ع����� */
        ov2640_brightness(l2345sel[1]);         /* �������� */
        ov2640_color_saturation(l2345sel[2]);   /* ɫ������ */
        ov2640_contrast(l2345sel[3]);           /* �Աȶ����� */

        ov2640_rgb565_mode();   /* RGB565ģʽ */
        dcmi_init();            /* DCMI���� */
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA���� */
        ov2640_outsize_set(lcddev.width, lcddev.height);    /* ��������ͷ����ߴ�ΪLCD�ĳߴ��С */
        dcmi_start();           /* �������� */
        delay_ms(200);
        
        /* ��Ϊ����ͷHREF��PCLK��TPAD����ż����м�,��������ͷ��,���нϴ�Ӱ��,�����³�ʼ��TPAD */
//        OSTaskSuspend(3);   /* ����watch_task */
//        tpad_init(16);      /* ���³�ʼ��TPAD */
//        OSTaskResume(3);    /* �ָ�watch_task */
        
        delay_ms(200);
        system_task_return = 0; /* ���TPAD */
        jpeg_size = 12;         /* JPEG����,Ĭ������Ϊ1600*1200�ֱ���(UXGA) */

        while (1)
        {
            tp_dev.scan(0);

            if (tp_dev.sta & TP_PRES_DOWN)
            {
                dcmi_stop();
                sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
                caption = (uint8_t *)camera_l00fun_caption[gui_phy.language];
                res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 6) / 2, 160, 72 + 32 * 6, (uint8_t **)camera_l00fun_table[gui_phy.language], 6, (uint8_t *)&l00sel, 0X90, caption); /* ��ѡ */
                sw_ov2640_mode();   /* �л�ΪOV2640ģʽ */

                if (res == 0)
                {
                    dcmi_start();
                    delay_ms(200);
                    dcmi_stop();
                    sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
                    caption = (uint8_t *)camera_l00fun_table[gui_phy.language][l00sel];

                    switch (l00sel)
                    {
                        case 0:/* �������� */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 5) / 2, 160, 72 + 32 * 5, (uint8_t **)camera_l10fun_table[gui_phy.language], 5, (uint8_t *)&l10sel, 0X90, caption); /* ��ѡ */
                            sw_ov2640_mode();/* �л�ΪOV2640ģʽ */

                            if (res == 0)
                            {
                                ov2640_light_mode(l10sel);
                            }

                            break;

                        case 1:/* ��Ч���� */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 7) / 2, 160, 72 + 32 * 7, (uint8_t **)camera_l11fun_table[gui_phy.language], 7, (uint8_t *)&l11sel, 0X90, caption); /* ��ѡ */
                            sw_ov2640_mode();/* �л�ΪOV2640ģʽ */

                            if (res == 0)
                            {
                                ov2640_special_effects(l11sel);
                            }

                            break;

                        case 2:/* �ع����� */
                        case 3:/* �������� */
                        case 4:/* ɫ������ */
                        case 5:/* �Աȶ����� */
                            selx = l2345sel[l00sel - 2]; /* �õ�֮ǰ��ѡ�� */
                            res = app_items_sel((lcddev.width - 160) / 2, (lcddev.height - 72 - 32 * 5) / 2, 160, 72 + 32 * 5, (uint8_t **)camera_l125fun_table[gui_phy.language], 5, (uint8_t *)&selx, 0X90, caption); /* ��ѡ */
                            sw_ov2640_mode();/* �л�ΪOV2640ģʽ */

                            if (res == 0)
                            {
                                l2345sel[l00sel - 2] = selx; /* ��¼��ֵ */

                                if (l00sel == 2)ov2640_auto_exposure(selx);     /* �Զ��ع����� */

                                if (l00sel == 3)ov2640_brightness(selx);        /* �������� */

                                if (l00sel == 4)ov2640_color_saturation(selx);  /* ɫ������ */

                                if (l00sel == 5)ov2640_contrast(selx);          /* �Աȶ����� */
                            }

                            break;
                    }
                }
                dcmi_start();
            }

            if (system_task_return)break; /* TPAD���� */

            key = key_scan(0); /* ����ɨ�� */

            if (key)
            {
                dcmi_stop();

                while (key_scan(1)); /* �ȴ������ɿ� */

                tcnt = 0;

                switch (key)
                {
                    case KEY0_PRES: /* KEY0����,JPEG���� */
                    case KEY2_PRES: /* KEY2����,BMP���� */
                        LED1(0);    /* DS1��,��ʾ������ */
                        sw_sdcard_mode();       /* �л�ΪSD��ģʽ */

                        if (key == KEY0_PRES)   /* JPEG���� */
                        {
                            camera_new_pathname(pname, OV2640_JPEG_MODE);   /* �õ�jpg�ļ��� */
                            res = ov2640_jpg_photo(pname);
                        }
                        else /* BMP���� */
                        {
                            camera_new_pathname(pname, OV2640_RGB565_MODE); /* �õ�bmp�ļ��� */
                            res = bmp_encode(pname, 0, 0, lcddev.width, lcddev.height, 0); /* bmp���� */
                        }

                        if (res) /* ����ʧ���� */
                        {
                            if (res == 0XFF)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[1][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* �ڴ���� */
                            else if (res == 0XFD)window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[2][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* ���ݴ��� */
                            else window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 100) / 2, 200, 100, (uint8_t *)camera_failmsg_tbl[0][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0); /* ��ʾSD���Ƿ���� */
                        }
                        else
                        {
                            strcpy((char *)psn, (const char *)camera_remind_tbl[2][gui_phy.language]);
                            strcat((char *)psn, (const char *)pname);
                            window_msg_box((lcddev.width - 180) / 2, (lcddev.height - 80) / 2, 180, 80, psn, (uint8_t *)camera_saveok_caption[gui_phy.language], 12, 0, 0, 0);
                            BEEP(1);        /* �������̽У���ʾ������� */
                            delay_ms(100);
                        }

                        sw_ov2640_mode();   /* �л�ΪOV2640ģʽ */
                        LED1(1);            /* DS1��,��ʾ������� */
                        BEEP(0);            /* �������̽� */
                        delay_ms(2000);

                        if (scale == 0)
                        {
                            ov2640_image_win_set((1600 - lcddev.width) / 2, (1200 - lcddev.height) / 2, lcddev.width, lcddev.height); /* 1:1��ʵ�ߴ� */
                            ov2640_outsize_set(lcddev.width, lcddev.height);
                        }
                        else
                        {
                            ov2640_image_win_set(0, 0, 1600, 1200); /* ȫ�ߴ����� */
                        }

                        ov2640_outsize_set(lcddev.width, lcddev.height);
                        system_task_return = 0; /* ���TPAD */
                        break;

                    case KEY1_PRES:     /* KEY1����,����/1:1��ʾ(������) */
                        scale = !scale;

                        if (scale == 0)ov2640_image_win_set((1600 - lcddev.width) / 2, (1200 - lcddev.height) / 2, lcddev.width, lcddev.height); /* 1:1��ʵ�ߴ� */
                        else ov2640_image_win_set(0, 0, 1600, 1200);    /* ȫ�ߴ����� */

                        ov2640_outsize_set(lcddev.width, lcddev.height);
                        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, (uint8_t *)camera_scalemsg_tbl[scale][gui_phy.language], (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);

                        while (key_scan(1) == 0 && tcnt < 80)           /* �ȴ�800ms,���û�а������µĻ� */
                        {
                            delay_ms(10);
                            tcnt++;
                        }

                        break;

                    case WKUP_PRES:/* �ֱ������� */
                        jpeg_size++;

                        if (jpeg_size > 12)jpeg_size = 0;

                        sprintf((char *)psn, "%s:%dX%d(%s)", camera_remind_tbl[3][gui_phy.language], camera_jpeg_img_size_tbl[jpeg_size][0], camera_jpeg_img_size_tbl[jpeg_size][1], camera_jpegimgsize_tbl[jpeg_size]);
                        window_msg_box((lcddev.width - 200) / 2, (lcddev.height - 80) / 2, 200, 80, psn, (uint8_t *)APP_REMIND_CAPTION_TBL[gui_phy.language], 12, 0, 0, 0);

                        while (key_scan(1) == 0 && tcnt < 80) /* �ȴ�800ms,���û�а������µĻ� */
                        {
                            delay_ms(10);
                            tcnt++;
                        }

                        break;

                }

                sw_ov2640_mode();   /* �л�ΪOV2640ģʽ */
                dcmi_start();
            }

            if (hsync_int)          /* �ող���֡�ж�,������ʱ */
            {
                delay_ms(10);
                hsync_int = 0;
            }
        }
    }

    dcmi_stop();        /* ֹͣ����ͷ���� */
    sw_sdcard_mode();   /* �л�ΪSD��ģʽ */
    gui_memin_free(pname);
    gui_memin_free(psn);
    tpad_init(8);       /* ���³�ʼ��TPAD */
    g_tpad_default_val = tpad_temp_val; /* �ָ�ԭ����ֵ */
    return 0;
}















