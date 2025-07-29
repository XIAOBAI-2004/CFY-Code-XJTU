/**
 ****************************************************************************************************
 * @file        mjpeg.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-18
 * @brief       MJPEG视频处理 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220118
 * 第一次发布
 *
 ****************************************************************************************************
 */
 
#include "./MJPEG/mjpeg.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/source/ff.h"
#include "./BSP/LCD/lcd.h"


/* 简单快速的内存分配,以提高速度 */
#define MJPEG_MAX_MALLOC_SIZE       38 * 1024       /* 最大可以分配38K字节 */


struct jpeg_decompress_struct *p_cinfo;
struct my_error_mgr *p_jerr;
uint8_t *p_jpegbuf;             /* jpeg数据缓存指针 */
uint8_t *p_jmembuf;             /* mjpeg解码的 内存池 */
uint8_t *p_jmembuf;             /* mjpeg解码的 内存池 */

uint32_t g_jbufsize;            /* jpeg buf大小 */
uint16_t g_imgoffx, g_imgoffy;  /* 图像在x,y方向的偏移量 */
uint8_t *p_jmembuf;             /* mjpeg解码的 内存池 */
uint32_t g_jmempos;             /* 内存池指针 */


/**
 * @brief       mjpeg申请内存
 * @param       num   : 下一次分配的地址起始地址
 * @retval      无
 */
void *mjpeg_malloc(uint32_t num)
{
    uint32_t curpos = g_jmempos;    /* 此次分配的起始地址 */
    g_jmempos += num;               /* 下一次分配的起始地址 */

    if (g_jmempos > 38 * 1024)
    {
        printf("mem error:%d,%d", curpos, num);
    }

    return (void *)&p_jmembuf[curpos];  /* 返回申请到的内存首地址 */
}

/**
 * @brief       错误退出
 * @param       p_cinfo   : JPEG编码解码控制结构体
 * @retval      无
 */
static void my_error_exit(j_common_ptr p_cinfo)
{
    my_error_ptr myerr = (my_error_ptr) p_cinfo->err;
    (*p_cinfo->err->output_message) (p_cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

/**
 * @brief       发出消息
 * @param       p_cinfo     : JPEG编码解码控制结构体
 * @param       msg_level   : 消息等级
 * @retval      无
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
 * @brief       初始化资源,不执行任何操作
 * @param       p_cinfo   : JPEG编码解码控制结构体
 * @retval      无
 */
static void init_source(j_decompress_ptr p_cinfo)
{
    /* 不需要做任何事情. */
    return;
}

/**
 * @brief       填充输入缓冲区,一次性读取整帧数据
 * @param       p_cinfo   : JPEG编码解码控制结构体
 * @retval      无
 */
static boolean fill_input_buffer(j_decompress_ptr p_cinfo)
{
    if (g_jbufsize == 0) /* 结束了 */
    {
        printf("jd read off\r\n");
        p_jpegbuf[0] = (uint8_t) 0xFF;          /* 填充结束符 */
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
 * @brief       在文件里面,跳过num_bytes个数据
 * @param       p_cinfo       : JPEG编码解码控制结构体
 * @param       num_bytes   : 跳过字节长度
 * @retval      无
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
 * @brief       在解码结束后,被jpeg_finish_decompress函数调用
 * @param       p_cinfo       : JPEG编码解码控制结构体
 * @retval      无
 */
static void term_source(j_decompress_ptr p_cinfo)
{
    /* 不做任何处理 */
    return;
}

/**
 * @brief       初始化jpeg解码数据源
 * @param       p_cinfo   : JPEG编码解码控制结构体
 * @retval      无
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
 * @brief       初始化jpeg解码数据源
 * @param       offx   : x方向的偏移
 * @param       offy   : y方向的偏移
 * @retval      0,成功;
 *              1,失败
 */
uint8_t mjpegdec_init(uint16_t offx, uint16_t offy)
{
    p_cinfo = mymalloc(SRAMCCM, sizeof(struct jpeg_decompress_struct));
    p_jerr = mymalloc(SRAMCCM, sizeof(struct my_error_mgr));
    p_jmembuf = mymalloc(SRAMCCM, MJPEG_MAX_MALLOC_SIZE);   /* MJPEG解码内存池申请 */

    if (p_cinfo == 0 || p_jerr == 0 || p_jmembuf == 0)
    {
        mjpegdec_free();
        return 1;
    }

    /* 保存图像在x,y方向的偏移量 */
    g_imgoffx = offx;
    g_imgoffy = offy;
    
    return 0;
}

/**
 * @brief       mjpeg结束,释放内存
 * @param       无
 * @retval      无
 */
void mjpegdec_free(void)
{
    myfree(SRAMCCM, p_cinfo);
    myfree(SRAMCCM, p_jerr);
    myfree(SRAMCCM, p_jmembuf);
}

/**
 * @brief       解码一副JPEG图片
 * @param       buf     : jpeg数据流数组
 * @param       bsize   : 数组大小
 * @retval      0,成功;
 *              其他,错误
 */
uint8_t mjpegdec_decode(uint8_t *buf, uint32_t bsize)
{
    JSAMPARRAY buffer;

    if (bsize == 0)return 1;

    p_jpegbuf = buf;
    g_jbufsize = bsize;
    g_jmempos = 0; /* MJEPG解码,重新从0开始分配内存 */

    p_cinfo->err = jpeg_std_error(&p_jerr->pub);
    p_jerr->pub.error_exit = my_error_exit;
    p_jerr->pub.emit_message = my_emit_message;

    //if(bsize>20*1024)printf("s:%d\r\n",bsize);
    if (setjmp(p_jerr->setjmp_buffer))    /* 错误处理 */
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
    lcd_write_ram_prepare();        /* 开始写入GRAM */

    while (p_cinfo->output_scanline < p_cinfo->output_height)
    {
        jpeg_read_scanlines(p_cinfo, buffer, 1);
    }

    lcd_set_window(0, 0, lcddev.width, lcddev.height);  /* 恢复窗口 */
    jpeg_finish_decompress(p_cinfo);
    jpeg_destroy_decompress(p_cinfo);
    
    return 0;
}
















