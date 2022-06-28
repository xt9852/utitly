/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_utitly.c
 *\author       xt
 *\version      1.0.0
 *\date         2016.12.07
 *\brief        工具模块实现,UTF-8(No BOM)
 */
#include "xt_utitly.h"
#include <stdio.h>

#ifdef _WINDOWS

#include <time.h>
#include <stdint.h>
#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

/**
 *\brief        得到精确时间,微秒级
 *\param[out]   tv          时间
 *\param[in]    tz          时区
 *\return       0           成功
 */
int gettimeofday(struct timeval *tv, void *tz)
{
    if (NULL == tv)
    {
        return -1;
    }

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);   // 得到1601年1月1日的100纳秒数

    // 得到微秒
    uint64_t ms = ((uint64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10 - DELTA_EPOCH_IN_MICROSECS;

    tv->tv_sec  = (long)(ms / 1000000UL);  // 秒
    tv->tv_usec = (long)(ms % 1000000UL);  // 微秒
    return 0;
}

#else
#include <sys/time.h>
#endif

/**
 *\brief        得到格式化后的信息
 *\param[in]    n            数据
 *\param[out]   info            信息
 *\param[in]    info_size       缓冲区大小
 *\return                       无
 */
void format_data(unsigned __int64 n, char *info, int size)
{
    double g = (double)n / (1024.0 * 1024 * 1024);
    double m = (double)n / (1024.0 * 1024);
    double k = (double)n / (1024.0);

    if (g >= 0.9)
    {
        snprintf(info, size, "%.2fG", g);
    }
    else if (m >= 0.9)
    {
        snprintf(info, size, "%.2fM", m);
    }
    else if (k >= 0.9)
    {
        snprintf(info, size, "%.2fK", k);
    }
    else
    {
        snprintf(info, size, "%I64u", n);
    }
}
