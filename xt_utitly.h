/**
 *\file     xt_utitly.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2016.12.07
 *\brief    工具模块定义
 */
#ifndef _XT_UTITLY_H_
#define _XT_UTITLY_H_

#ifndef bool
#define bool                unsigned char                                       ///< 布尔类型
#endif

#ifndef true
#define true                1                                                   ///< 真值
#endif

#ifndef TRUE
#define TRUE                1                                                   ///< 真值
#endif

#ifndef false
#define false               0                                                   ///< 假值
#endif

#ifndef FALSE
#define FALSE               0                                                   ///< 假值
#endif

#ifndef NULL
#define NULL                0                                                   ///< 空
#endif

#ifndef SIZEOF
#define SIZEOF(x)           sizeof(x)/sizeof(x[0])                              ///< 元素数量
#endif

#ifndef SP
#define SP(x, f, ...)       sprintf_s(x, sizeof(x) - 1, f, __VA_ARGS__)         ///< 格式化输出字符串
#endif

#ifdef _WINDOWS

// WINDOWS系统没有这些函数
#include <windows.h>
#include <process.h>

#define sleep(n)        Sleep(n*1000)                                           ///< 等待1秒
#define msleep(n)       Sleep(n)                                                ///< 等待1毫秒
#define getpid()        _getpid()                                               ///< 得到进程ID
#define gettid()        GetCurrentThreadId()                                    ///< 得到线程ID
#define strncasecmp     strnicmp                                                ///< 不区分大小写的比较
#define PATH_SEGM       '\\'                                                    ///< WINDOWS路径分割符

/**
 *\brief                    得到精确时间,微秒级
 *\param[out]   tv          时间
 *\param[in]    tz          时区
 *\return       0           成功
 */
int gettimeofday(struct timeval *tv, void *tz);

#else

#define PATH_SEGM       '/'                                                     ///< LINUX路径分割符

#endif // _WINDOWS

/**
 *\brief                    得到格式化后的信息
 *\param[in]    n           数据
 *\param[out]   info        信息
 *\param[in]    size        缓冲区大小
 *\return                   无
 */
void format_data(unsigned __int64 n, char *info, int size);

#endif
