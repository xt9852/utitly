/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_log.h
 *\author       xt
 *\version      1.0.0
 *\date         2016.12.07
 *\brief        日志模块定义,UTF-8(No BOM)
 */
#ifndef _XT_LOG_H_
#define _XT_LOG_H_
#include <stdio.h>
#ifdef _WINDOWS
#include <windows.h>
#include <process.h>
#define close           closesocket
#define sleep(n)        Sleep(n*1000)
#define msleep(n)       Sleep(n)
#define getpid()        _getpid()
#define gettid()        GetCurrentThreadId()
#define strncasecmp     strnicmp
#define PATH_SEGM       '\\'
#define D(l, ...)       log_write(l, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define DBG(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define MSG(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  __VA_ARGS__)
#define WAR(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  __VA_ARGS__)
#define ERR(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define PATH_SEGM       '/'
#define DBG (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, ##args)
#define MSG (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  ##args)
#define WAR (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  ##args)
#define ERR (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, ##args)
#endif

#ifndef bool
#define bool  unsigned char
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef SIZEOF
#define SIZEOF(x)   sizeof(x)/sizeof(x[0])
#endif

/// 日志级别
enum
{
    LOG_LEVEL_DEBUG,                ///< 调试
    LOG_LEVEL_INFO,                 ///< 信息
    LOG_LEVEL_WARN,                 ///< 警告
    LOG_LEVEL_ERROR                 ///< 错误
};

/// 日志文件保留周期
enum
{
    LOG_CYCLE_MINUTE,               ///< 分钟
    LOG_CYCLE_HOUR,                 ///< 时
    LOG_CYCLE_DAY,                  ///< 天
    LOG_CYCLE_WEEK,                 ///< 周
};

/// 日志信息
typedef struct _xt_log
{
    char            filename[512];  ///< 日志文件名
    int             level;          ///< 日志级别(调试,信息,警告,错误)
    int             cycle;          ///< 日志文件保留周期(时,天,周)
    int             backup;         ///< 日志文件保留数量
    bool            clean;          ///< 首次打开日志文件时是否清空文件内容

    bool            run;            ///< 日志线程是否运行
    FILE*           file;           ///< 日志文件句柄

} xt_log, *p_xt_log;                ///< 日志信息指针

extern xt_log g_log;                ///< 全局日志指针

/**
 *\brief        初始化日志
 *\param[in]    log         日志数据,需要filename,level,cycle,backup,clean
 *\return       0           成功
 */
int log_init(p_xt_log log);

/**
 *\brief        反初始化日志
 *\param[in]    log         日志数据
 *\return       无
 */
void log_uninit(p_xt_log log);

/**
 *\brief        写日志
 *\param[in]    log         日志数据
 *\param[in]    file        文件名
 *\param[in]    func        函数名
 *\param[in]    line        行号
 *\param[in]    level       日志级别
 *\param[in]    fmt         日志内容
 *\return                   无
 */
void log_write(p_xt_log log, const char *file, const char *func, int line, int level, const char *fmt, ...);

#endif
