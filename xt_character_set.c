/**
 *\file     xt_character_set.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.06.04
 *\brief    字符集转码实现
 */
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "xt_character_set.h"

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

/**
 *\brief                        将utf8转成unicode
 *\param[in]    src             源串
 *\param[in]    src_len         源串字节长
 *\param[out]   dst             目标串
 *\param[out]   dst_short_len   输入缓冲区short大小,输出目标串short长
 *\return       0               成功
 */
int utf8_unicode(const unsigned char *src, unsigned int src_len, unsigned short *dst, unsigned int *dst_short_len)
{
    if (NULL == src || 0 == src_len || NULL == dst || NULL == dst_short_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    // 转成unicode后的short类型长度
    unsigned int len = MultiByteToWideChar(CP_UTF8, 0, src, src_len, NULL, 0);

    if (len >= *dst_short_len)
    {
        E("too short\n");
        return -2;
    }

    // 转成unicode
	MultiByteToWideChar(CP_UTF8, 0, src, src_len, dst, *dst_short_len);

    dst[len] = L'\0';

    *dst_short_len = len;
	return 0;
}

/**
 *\brief                    将unicode转成gbk
 *\param[in]    src         源串
 *\param[in]    src_len     源串short长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     输入缓冲区字节大小,输出目标串字节长
 *\return       0           成功
 */
int unicode_gbk(const unsigned short *src, unsigned int src_short_len, unsigned char *dst, unsigned int *dst_len)
{
    if (NULL == src || 0 == src_short_len || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    unsigned int len = WideCharToMultiByte(CP_ACP, 0, src, src_short_len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short\n");
        return -2;
    }

    WideCharToMultiByte(CP_ACP, 0, src, src_short_len, dst, *dst_len, 0, 0);

    dst[len] = '\0';

    *dst_len = len;
    return 0;
}

/**
 *\brief                        将unicode转成utf8
 *\param[in]    src             源串
 *\param[in]    src_short_len   源串short长
 *\param[out]   dst             目标串
 *\param[out]   dst_len         输入缓冲区字节大小,输出目标串字节长
 *\return       0               成功
 */
int unicode_utf8(const unsigned short *src, unsigned int src_short_len, unsigned char *dst, unsigned int *dst_len)
{
    if (NULL == src || 0 == src_short_len || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    unsigned int len = WideCharToMultiByte(CP_UTF8, 0, src, src_short_len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short\n");
        return -2;
    }

    WideCharToMultiByte(CP_UTF8, 0, src, src_short_len, dst, *dst_len, 0, 0);

    dst[len] = '\0';

    *dst_len = len;
    return 0;
}

/**
 *\brief                    将gbk转成utf8
 *\param[in]    src         源串
 *\param[in]    src_len     源串字节大长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     输入缓冲区字节大小,输出目标串字节长
 *\return       0           成功
 */
int gbk_utf8(const unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    if (NULL == src || 0 == src_len || NULL == dst || NULL == dst_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    // 转成unicode后的长度
    unsigned int len = MultiByteToWideChar(CP_ACP, 0, src, src_len, NULL, 0);

    if (len >= *dst_len)
    {
        E("too short\n");
        return -2;
    }

    unsigned short *tmp = (unsigned short*)malloc(len * 2 + 2);

    // 转成unicode
	MultiByteToWideChar(CP_ACP, 0, src, src_len, tmp, len);

    tmp[len] = L'\0';

    // 转成utf8后的长度
    len = WideCharToMultiByte(CP_UTF8, 0, tmp, len, 0, 0, 0, 0);

    if (len >= *dst_len)
    {
        E("too short\n");
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
