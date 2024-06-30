/**
 *\file     xt_pinyin.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.02.08
 *\brief    拼音模块实现
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#ifdef XT_LOG
    #include "xt_log.h"
#else
    #include <stdio.h>
    #include <stdlib.h>
#ifdef _WINDOWS
    #define D(...)      printf(__VA_ARGS__)
    #define I(...)      printf(__VA_ARGS__)
    #define W(...)      printf(__VA_ARGS__)
    #define E(...)      printf(__VA_ARGS__)
#else
    #define D(args...)  printf(args)
    #define I(args...)  printf(args)
    #define W(args...)  printf(args)
    #define E(args...)  printf(args)
#endif
#endif

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true    1
#endif

#ifndef false
#define false   0
#endif

bool            g_pinyin_malloc = false;    ///< 拼音数据是于malloc分配的

unsigned char  *g_pinyin        = NULL;     ///< 拼音数据

typedef struct _xt_peyin                    ///  拼音组数据
{
    char        *m;                         ///< 拼音字母
    int         len;                        ///< 拼音长

} xt_peyin, *p_xt_peyin;                    ///< 拼音组数据指针

const xt_peyin g_pinyin_sm[] = {            ///< 拼音声母
    { "",   1},
    { "ch", 2},
    { "sh", 2},
    { "zh", 2},
    { "b",  1},
    { "c",  1},
    { "d",  1},
    { "f",  1},
    { "g",  1},
    { "h",  1},
    { "j",  1},
    { "k",  1},
    { "l",  1},
    { "m",  1},
    { "n",  1},
    { "p",  1},
    { "q",  1},
    { "r",  1},
    { "s",  1},
    { "t",  1},
    { "w",  1},
    { "x",  1},
    { "y",  1},
    { "z",  1}
};

const xt_peyin g_pinyin_ym[] = {            ///< 拼音韵母
    { "",     1},
    { "iang", 4},
    { "iong", 4},
    { "uang", 4},
    { "ueng", 4},
    { "ang",  3},
    { "eng",  3},
    { "ian",  3},
    { "iao",  3},
    { "ing",  3},
    { "ong",  3},
    { "uai",  3},
    { "uan",  3},
    { "uei",  3},
    { "uen",  3},
    { "ai",   2},
    { "an",   2},
    { "ao",   2},
    { "ei",   2},
    { "en",   2},
    { "ia",   2},
    { "ie",   2},
    { "in",   2},
    { "iu",   2},
    { "ou",   2},
    { "ua",   2},
    { "uo",   2},
    { "ue",   2},   // üe
    { "un",   2},   // ün
    { "a",    1},
    { "e",    1},
    { "i",    1},
    { "o",    1},
    { "u",    1},
};

/**
 *\brief                    从资源中加载拼音数据
 *\param[in]    res_type    资源类名,"PINYIN"
 *\param[in]    res_id      资源ID
 *\return       0           成功
 */
int pinyin_init_res(char *res_type, int res_id)
{
    if (NULL == res_type)
    {
        printf("%s type is null\n", __FUNCTION__);
        return -1;
    }

    HRSRC res = FindResourceA(NULL, MAKEINTRESOURCEA(res_id), res_type);

    if (NULL == res)
	{
        printf("%s FindResource fail id:%d\n", __FUNCTION__, res_id);
        return -2;
    }

    // 加载资源到内存
    HGLOBAL res_global = LoadResource(NULL, res);

    if (NULL == res_global)
	{
        printf("%s LoadResource fail\n", __FUNCTION__);
        return -3;
    }

    // 内存是由malloc分配的
    if (g_pinyin_malloc)
    {
        free(g_pinyin);
    }

    // 锁定资源内存
    g_pinyin = LockResource(res_global);

    if (NULL == g_pinyin)
	{
        printf("%s LockResource fail\n", __FUNCTION__);
        return -4;
    }

    g_pinyin_malloc = false;

    D("ok");
    return 0;
}

/**
 *\brief                    从文件中加载拼音数据
 *\param[in]    filename    文件名
 *\return       0           成功
 */
int pinyin_init(const char *filename)
{
    if (NULL == filename)
    {
        D("%s filename is null\n", filename);
        return -1;
    }

    FILE *fp = NULL;
    fopen_s(&fp, filename, "rb");

    if (NULL == fp)
    {
        D("open file:%s error:%d\n", filename, GetLastError());
        return -1;
    }

    fseek(fp, 0, SEEK_END);

    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 内存是由malloc分配的
    if (g_pinyin_malloc)
    {
        free(g_pinyin);
    }

    g_pinyin = malloc(size);
    fread(g_pinyin, 1, size, fp);

    fclose(fp);

    g_pinyin_malloc = true;

    D("ok\n");
    return 0;
}

/**
 *\brief                    将gbk转成拼音
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int gbk_pinyin(const unsigned char *src, unsigned int src_len, char *dst, unsigned int *dst_len)
{
    if (NULL == src || NULL == dst || NULL == dst_len)
    {
        printf("%s src,dst,dst_len is null\n", __FUNCTION__);
        return -1;
    }

    if (NULL == g_pinyin)
    {
        printf("%s pinyin data is null\n", __FUNCTION__);
        return -2;
    }

    unsigned char *end  = dst + *dst_len;
    unsigned char *tail = dst;
    unsigned char  sm_value;
    unsigned char  ym_value;
    int            buff_pos;

    for (unsigned int i = 0; i < src_len; )
    {
        // GBK/2, GBK/3, GBK/4
        if ((src[i] >= 0xb0 && src[i] <= 0xf7 && src[i + 1] >= 0xa1 && src[i + 1] <= 0xfe) ||
            (src[i] >= 0x81 && src[i] <= 0xa0 && src[i + 1] >= 0x40 && src[i + 1] <= 0xfe) ||
            (src[i] >= 0xaa && src[i] <= 0xfe && src[i + 1] >= 0x40 && src[i + 1] <= 0xa0))
        {
            buff_pos = ((src[i] - 0x81) * (0xfe - 0x40 + 1) + (src[i + 1] - 0x40)) * 2;

            sm_value = g_pinyin[buff_pos];
            ym_value = g_pinyin[buff_pos + 1];

            D("%c%c=0x%02x%02x pos:%5d sm=%02d ym=%02d pinyin:%s%s\n",
               src[i], src[i + 1], src[i], src[i + 1], buff_pos,
               sm_value, ym_value, g_pinyin_sm[sm_value].m, g_pinyin_ym[ym_value].m);

            if ((tail + g_pinyin_sm[sm_value].len) >= end)
            {
                return -3;
            }

            strcpy_s(tail, end - tail, g_pinyin_sm[sm_value].m);

            tail += g_pinyin_sm[sm_value].len;

            if ((tail + g_pinyin_ym[ym_value].len) >= end)
            {
                return -4;
            }

            strcpy_s(tail, end - tail, g_pinyin_ym[ym_value].m);

            tail += g_pinyin_ym[ym_value].len;

            i += 2;
        }
        else if (src[i] >= 0x80)
        {
            if ((tail + 2) >= end)
            {
                return -5;
            }

            D("%c%c=0x%02x%02x i:%d\n", src[i], src[i + 1], src[i], src[i + 1], i);

            *tail++ = src[i];
            *tail++ = src[i + 1];

            i += 2;
        }
        else
        {
            if ((tail + 1) >= end)
            {
                return -6;
            }

            D("%c=0x%02x i:%d\n", src[i], src[i],i);

            *tail++ = src[i];

            i++;
        }
    }

    *dst_len = tail - dst;

    return 0;
}
