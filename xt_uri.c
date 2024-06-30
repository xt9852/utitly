/**
 *\file     xt_uri.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022-02-08
 *\brief    HTTP模块实现
 */
#include <stdlib.h>
#include <string.h>
#include "xt_uri.h"

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

const static char HEX_STR[] = "0123456789ABCDEF";                               ///< 十六进制字符

/**
 *\brief                    十六进制字符转数字
 *\param[in]    ch          十六进制字符
 *\return                   数字
 */
char http_to_hex(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    else
    {
        E("ch:0x%02x\n", ch);
        return -1;
    }
}

/**
 *\brief                    URI编码
 *\param[in]    in          原始的数据
 *\param[in]    in_len      原始的数据长度
 *\param[out]   out         编码后数据
 *\param[out]   out_len     输入数据缓冲区大小,输出编码后数据长度
 *\return       0           成功
 */
int uri_encode(const char *in, unsigned int in_len, char *out, unsigned int *out_len)
{
    if (NULL == in || 0 == in_len || NULL == out || NULL == out_len || *out_len < in_len * 3)
    {
        E("param null or buff too small\n");
        return -1;
    }

    unsigned int j = 0;
    unsigned char hex;

    for (unsigned int i = 0; i < in_len; i++)
    {
        if ((in[i] >= '0' && in[i] <= '9') || (in[i] >= 'a' && in[i] <= '9') || (in[i] >= 'A' && in[i] <= 'Z'))
        {
            out[j++] = in[i];
        }
        else
        {
            hex = in[i];
            out[j++] = '%';
            out[j++] = HEX_STR[hex / 16];
            out[j++] = HEX_STR[hex % 16];
        }
    }

    out[j] = '\0';
    *out_len = j;

    return 0;
}

/**
 *\brief                    URI解码
 *\param[in]    in          URI数据
 *\param[in]    in_len      URI数据长度
 *\param[out]   out         原始数据
 *\param[out]   out_len     输入数据缓冲区大小,输出解码后数据长度
 *\return       0           成功
 */
int uri_decode(const char *in, unsigned int in_len, char *out, unsigned int *out_len)
{
    if (NULL == in || 0 == in_len || NULL == out || NULL == out_len || *out_len < in_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    int   ret = 0;
    int   len = 0;
    char  hex[2];
    const char *beg = in;
    const char *ptr = NULL;

    while (ptr = strchr(beg, '%'))
    {
        hex[0] = http_to_hex(ptr[1]);
        hex[1] = http_to_hex(ptr[2]);

        if (hex[0] < 0 || hex[1] < 0)
        {
            out[len++] = *beg++;
            continue;
        }

        memcpy(out + len, beg, ptr - beg);
        len += ptr - beg;
        out[len++] = (hex[0] << 4) + hex[1];
        beg = ptr + 3;
    }

    if ('\0' != *beg)
    {
        strcpy_s(out + len, *out_len - len, beg);
        len += strlen(beg);
    }
    else
    {
        out[len] = '\0';
    }

    *out_len = len;
    return ret;
}
