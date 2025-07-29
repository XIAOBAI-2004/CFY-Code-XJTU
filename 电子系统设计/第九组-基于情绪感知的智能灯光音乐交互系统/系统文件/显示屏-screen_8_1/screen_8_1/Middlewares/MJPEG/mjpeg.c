/**
 ****************************************************************************************************
 * @file        mjpeg.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-18
 * @brief       MJPEG��Ƶ���� ����
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220118
 * ��һ�η���
 *
 ****************************************************************************************************
 */
 
#include "./MJPEG/mjpeg.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/source/ff.h"
#include "./BSP/LCD/lcd.h"


/* �򵥿��ٵ��ڴ����,������ٶ� */
#define MJPEG_MAX_MALLOC_SIZE       38 * 1024       /* �����Է���38K�ֽ� */


struct jpeg_decompress_struct *p_cinfo;
struct my_error_mgr *p_jerr;
uint8_t *p_jpegbuf;             /* jpeg���ݻ���ָ�� */
uint8_t *p_jmembuf;             /* mjpeg����� �ڴ�� */
uint8_t *p_jmembuf;             /* mjpeg����� �ڴ�� */

uint32_t g_jbufsize;            /* jpeg buf��С */
uint16_t g_imgoffx, g_imgoffy;  /* ͼ����x,y�����ƫ���� */
uint8_t *p_jmembuf;             /* mjpeg����� �ڴ�� */
uint32_t g_jmempos;             /* �ڴ��ָ�� */


/**
 * @brief       mjpeg�����ڴ�
 * @param       num   : ��һ�η���ĵ�ַ��ʼ��ַ
 * @retval      ��
 */
void *mjpeg_malloc(uint32_t num)
{
    uint32_t curpos = g_jmempos;    /* �˴η������ʼ��ַ */
    g_jmempos += num;               /* ��һ�η������ʼ��ַ */

    if (g_jmempos > 38 * 1024)
    {
        printf("mem error:%d,%d", curpos, num);
    }

    return (void *)&p_jmembuf[curpos];  /* �������뵽���ڴ��׵�ַ */
}

/**
 * @brief       �����˳�
 * @param       p_cinfo   : JPEG���������ƽṹ��
 * @retval      ��
 */
static void my_error_exit(j_common_ptr p_cinfo)
{
    my_error_ptr myerr = (my_error_ptr) p_cinfo->err;
    (*p_cinfo->err->output_message) (p_cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

/**
 * @brief       ������Ϣ
 * @param       p_cinfo     : JPEG���������ƽṹ��
 * @param       msg_level   : ��Ϣ�ȼ�
 * @retval      ��
 */
METHODDEF(void) my_emit_message(j_common_ptr p_cinfo, int msg_level)
{
    my_error_ptr myerr = (my_error_ptr) p_cinfo->err;

    if (msg_level < 0)
    {
        printf("emit msg:%d\r\n", msg_level);
        longjmp(myerr->setjmp_buffer, 1);
    }
}

/**
 * @brief       ��ʼ����Դ,��ִ���κβ���
 * @param       p_cinfo   : JPEG���������ƽṹ��
 * @retval      ��
 */
static void init_source(j_decompress_ptr p_cinfo)
{
    /* ����Ҫ���κ�����. */
    return;
}

/**
 * @brief       ������뻺����,һ���Զ�ȡ��֡����
 * @param       p_cinfo   : JPEG���������ƽṹ��
 * @retval      ��
 */
static boolean fill_input_buffer(j_decompress_ptr p_cinfo)
{
    if (g_jbufsize == 0) /* ������ */
    {
        printf("jd read off\r\n");
        p_jpegbuf[0] = (uint8_t) 0xFF;          /* �������� */
        p_jpegbuf[1] = (uint8_t) JPEG_EOI;
        p_cinfo->src->next_input_byte = p_jpegbuf;
        p_cinfo->src->bytes_in_buffer = 2;
    }
    else
    {
        p_cinfo->src->next_input_byte = p_jpegbuf;
        p_cinfo->src->bytes_in_buffer = g_jbufsize;
        g_jbufsize -= g_jbufsize;
    }

    return TRUE;
}

/**
 * @brief       ���ļ�����,����num_bytes������
 * @param       p_cinfo       : JPEG���������ƽṹ��
 * @param       num_bytes   : �����ֽڳ���
 * @retval      ��
 */
static void skip_input_data(j_decompress_ptr p_cinfo, long num_bytes)
{
    /* Just a dumb implementation for now.  Could use fseek() except
    * it doesn't work on pipes.  Not clear that being smart is worth
    * any trouble anyway --- large skips are infrequent.
    */
    if (num_bytes > 0)
    {
        while (num_bytes > (long) p_cinfo->src->bytes_in_buffer)
        {
            num_bytes -= (long)p_cinfo->src->bytes_in_buffer;
            (void)p_cinfo->src->fill_input_buffer(p_cinfo);
            /* note we assume that fill_input_buffer will never
            * return FALSE, so suspension need not be handled.
            */
        }

        p_cinfo->src->next_input_byte += (size_t) num_bytes;
        p_cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

/**
 * @brief       �ڽ��������,��jpeg_finish_decompress��������
 * @param       p_cinfo       : JPEG���������ƽṹ��
 * @retval      ��
 */
static void term_source(j_decompress_ptr p_cinfo)
{
    /* �����κδ��� */
    return;
}

/**
 * @brief       ��ʼ��jpeg��������Դ
 * @param       p_cinfo   : JPEG���������ƽṹ��
 * @retval      ��
 */
static void jpeg_filerw_src_init(j_decompress_ptr p_cinfo)
{
    if (p_cinfo->src == NULL)     /* first time for this JPEG object? */
    {
        p_cinfo->src = (struct jpeg_source_mgr *)
                     (*p_cinfo->mem->alloc_small)((j_common_ptr) p_cinfo, JPOOL_PERMANENT,
                                                sizeof(struct jpeg_source_mgr));
    }

    p_cinfo->src->init_source = init_source;
    p_cinfo->src->fill_input_buffer = fill_input_buffer;
    p_cinfo->src->skip_input_data = skip_input_data;
    p_cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
    p_cinfo->src->term_source = term_source;
    p_cinfo->src->bytes_in_buffer = 0;    /* forces fill_input_buffer on first read */
    p_cinfo->src->next_input_byte = NULL; /* until buffer loaded */
}

/**
 * @brief       ��ʼ��jpeg��������Դ
 * @param       offx   : x�����ƫ��
 * @param       offy   : y�����ƫ��
 * @retval      0,�ɹ�;
 *              1,ʧ��
 */
uint8_t mjpegdec_init(uint16_t offx, uint16_t offy)
{
    p_cinfo = mymalloc(SRAMCCM, sizeof(struct jpeg_decompress_struct));
    p_jerr = mymalloc(SRAMCCM, sizeof(struct my_error_mgr));
    p_jmembuf = mymalloc(SRAMCCM, MJPEG_MAX_MALLOC_SIZE);   /* MJPEG�����ڴ������ */

    if (p_cinfo == 0 || p_jerr == 0 || p_jmembuf == 0)
    {
        mjpegdec_free();
        return 1;
    }

    /* ����ͼ����x,y�����ƫ���� */
    g_imgoffx = offx;
    g_imgoffy = offy;
    
    return 0;
}

/**
 * @brief       mjpeg����,�ͷ��ڴ�
 * @param       ��
 * @retval      ��
 */
void mjpegdec_free(void)
{
    myfree(SRAMCCM, p_cinfo);
    myfree(SRAMCCM, p_jerr);
    myfree(SRAMCCM, p_jmembuf);
}

/**
 * @brief       ����һ��JPEGͼƬ
 * @param       buf     : jpeg����������
 * @param       bsize   : �����С
 * @retval      0,�ɹ�;
 *              ����,����
 */
uint8_t mjpegdec_decode(uint8_t *buf, uint32_t bsize)
{
    JSAMPARRAY buffer;

    if (bsize == 0)return 1;

    p_jpegbuf = buf;
    g_jbufsize = bsize;
    g_jmempos = 0; /* MJEPG����,���´�0��ʼ�����ڴ� */

    p_cinfo->err = jpeg_std_error(&p_jerr->pub);
    p_jerr->pub.error_exit = my_error_exit;
    p_jerr->pub.emit_message = my_emit_message;

    //if(bsize>20*1024)printf("s:%d\r\n",bsize);
    if (setjmp(p_jerr->setjmp_buffer))    /* ������ */
    {
        jpeg_abort_decompress(p_cinfo);
        jpeg_destroy_decompress(p_cinfo);
        return 2;
    }

    jpeg_create_decompress(p_cinfo);
    jpeg_filerw_src_init(p_cinfo);
    jpeg_read_header(p_cinfo, TRUE);
    p_cinfo->dct_method = JDCT_IFAST;
    p_cinfo->do_fancy_upsampling = 0;
    jpeg_start_decompress(p_cinfo);
    lcd_set_window(g_imgoffx, g_imgoffy, p_cinfo->output_width, p_cinfo->output_height);
    lcd_write_ram_prepare();        /* ��ʼд��GRAM */

    while (p_cinfo->output_scanline < p_cinfo->output_height)
    {
        jpeg_read_scanlines(p_cinfo, buffer, 1);
    }

    lcd_set_window(0, 0, lcddev.width, lcddev.height);  /* �ָ����� */
    jpeg_finish_decompress(p_cinfo);
    jpeg_destroy_decompress(p_cinfo);
    
    return 0;
}
















