/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_log.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 日志模块接口定义
*************************************************/

#ifndef _XT_LOG_H_
#define _XT_LOG_H_

#ifdef _WINDOWS
#include <windows.h>
#include <process.h>
#define close           closesocket
#define sleep(n)        Sleep(n*1000)
#define getpid()        _getpid()
#define gettid()        GetCurrentThreadId()
#define strncasecmp     strnicmp
#define PATH_SEGM       '\\'

#define DBG(...) xt_log_write_lock( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define MSG(...) xt_log_write_lock( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  __VA_ARGS__)
#define WAR(...) xt_log_write_lock( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  __VA_ARGS__)
#define ERR(...) xt_log_write_lock( __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)

#else
#define PATH_SEGM       '/'
#define DBG (args...) xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, ##args)
#define MSG (args...) xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  ##args)
#define WAR (args...) xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  ##args)
#define ERR (args...) xt_log_write(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, ##args)
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

enum
{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
};


/**
 *\brief      加载配置文件
 *\param[in]  const char    *path  日志文件所在目录
 *\param[in]  void          *json  配置文件json根
 *\return     0-成功
 */
int xt_log_parse_config(const char *path, void *json);

/**
 *\brief      初始化日志
 *\param[in]  p_log_info info   信息
 *\return     0-成功
 */
int xt_log_init();

/**
 *\brief      反初始化日志
 *\return     无
 */
void xt_log_uninit();

/**
 *\brief      写日志不带锁
 *\param[in]  const char *file  文件名
 *\param[in]  const char *func  函数名
 *\param[in]  int line          行号
 *\param[in]  int level         日志级别
 *\param[in]  const char *fmt   日志内容
 *\return     无
 */
void xt_log_write(const char *file, const char *func, int line, int level, const char *fmt, ...);

/**
 *\brief      写日志带锁
 *\param[in]  const char *file  文件名
 *\param[in]  const char *func  函数名
 *\param[in]  int line          行号
 *\param[in]  int level         日志级别
 *\param[in]  const char *fmt   日志内容
 *\return     无
 */
void xt_log_write_lock(const char *file, const char *func, int line, int level, const char *fmt, ...);

#endif
