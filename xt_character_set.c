/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_character_set.c
 *\author       xt
 *\version      1.0.0
 *\date         2022.06.04
 *\brief        字符集转码实现,UTF-8(No BOM)
 */
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "xt_log.h"
#include "xt_character_set.h"

/**
 *\brief        将utf8转成unicode
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int utf8_unicode(const char *src, int src_len, short *dst, int *dst_len)
{
    if (NULL == src || src_len <= 0 || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small");
        return -1;
    }

    //D("src len:%d dst len:%d", src_len, *dst_len);

    // 转成unicode后的长度
    int len = MultiByteToWideChar(CP_UTF8, 0, src, src_len, NULL, 0);

    if (len >= *dst_len)
    {
        E("too short");
        return -2;
    }

    // 转成unicode
	MultiByteToWideChar(CP_UTF8, 0, src, src_len, dst, *dst_len);

    dst[len] = L'\0';

    *dst_len = len;
	return 0;
}

/**
 *\brief        将unicode转成ansi(gbk)
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int unicode_ansi(const short *src, int src_len, char *dst, int *dst_len)
{
    if (NULL == src || src_len <= 0 || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small");
        return -1;
    }

    //D("src len:%d dst len:%d", src_len, *dst_len);

    int len = WideCharToMultiByte(CP_ACP, 0, src, src_len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short");
        return -2;
    }

    WideCharToMultiByte(CP_ACP, 0, src, src_len, dst, *dst_len, 0, 0);

    dst[len] = '\0';

    *dst_len = len;
    return 0;
}

/**
 *\brief        将unicode转成utf8
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int unicode_utf8(const short *src, int src_len, char *dst, int *dst_len)
{
    if (NULL == src || src_len <= 0 || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small");
        return -1;
    }

    //D("src len:%d dst len:%d", src_len, *dst_len);

    int len = WideCharToMultiByte(CP_UTF8, 0, src, src_len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short");
        return -2;
    }

    WideCharToMultiByte(CP_UTF8, 0, src, src_len, dst, *dst_len, 0, 0);

    dst[len] = '\0';

    *dst_len = len;
    return 0;
}

/**
 *\brief        将ansi转成utf8
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int ansi_utf8(const char *src, int src_len, char *dst, int *dst_len)
{
    if (NULL == src || src_len <= 0 || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small");
        return -1;
    }

    //D("src len:%d dst len:%d", src_len, *dst_len);

    // 转成unicode后的长度
    int len = MultiByteToWideChar(CP_ACP, 0, src, src_len, NULL, 0);

    if (len >= *dst_len)
    {
        E("too short");
        return -2;
    }

    short *tmp = (short*)malloc(len * 2 + 2);

    // 转成unicode
	MultiByteToWideChar(CP_ACP, 0, src, src_len, tmp, len);

    tmp[len] = L'\0';

    // 转成utf8后的长度
    len = WideCharToMultiByte(CP_UTF8, 0, tmp, len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short");
        free(tmp);
        return -3;
    }

    // 转成utf8
    WideCharToMultiByte(CP_UTF8, 0, tmp, len, dst, *dst_len, 0, 0);

    dst[len] = '\0';

    *dst_len = len;

    free(tmp);
	return 0;
}
