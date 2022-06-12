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
#define DBG(...)        xt_log_write( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define MSG(...)        xt_log_write( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  __VA_ARGS__)
#define WAR(...)        xt_log_write( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  __VA_ARGS__)
#define ERR(...)        xt_log_write( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define PATH_SEGM       '/'
#define DBG (args...)   xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, ##args)
#define MSG (args...)   xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  ##args)
#define WAR (args...)   xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  ##args)
#define ERR (args...)   xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, ##args)
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

/// 日志级别
enum
{
    LOG_LEVEL_DEBUG = 'D',  ///< 调试
    LOG_LEVEL_INFO  = 'I',  ///< 信息
    LOG_LEVEL_WARN  = 'W',  ///< 警告
    LOG_LEVEL_ERROR = 'E'   ///< 错误
};

/**
 *\brief        初始化日志
 *\param[in]    path    日志文件所在目录
 *\param[in]    json    配置文件json根
 *\return       0       成功
 */
int xt_log_init(const char *path, void *json);

/**
 * \brief        反初始化日志
 * \return       无
 */
void xt_log_uninit();

/**
 *\brief        写日志不带锁
 *\param[in]    file    文件名
 *\param[in]    func    函数名
 *\param[in]    line    行号
 *\param[in]    level   日志级别
 *\param[in]    fmt     日志内容
 *\return               无
 */
void xt_log_write(const char *file, const char *func, int line, char level, const char *fmt, ...);

#endif
