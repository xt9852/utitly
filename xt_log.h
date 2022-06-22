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
#define close           closesocket             ///< 关闭端口
#define sleep(n)        Sleep(n*1000)           ///< 等待1秒
#define msleep(n)       Sleep(n)                ///< 等待1毫秒
#define getpid()        _getpid()               ///< 得到进程ID
#define gettid()        GetCurrentThreadId()    ///< 得到线程ID
#define strncasecmp     strnicmp                ///< 不区分大小写的比较
#define PATH_SEGM       '\\'                    ///< WINDOWS路径分割符
#define D(l, ...)       log_write(l, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)        ///< 指定文件输出
#define DBG(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)   ///< 调试
#define MSG(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  __VA_ARGS__)   ///< 信息
#define WAR(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  __VA_ARGS__)   ///< 警告
#define ERR(...)        log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)   ///< 错误
#else
#define PATH_SEGM       '/'                     ///< LINUX路径分割符
#define DBG (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, ##args)        ///< 调试
#define MSG (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO,  ##args)        ///< 信息
#define WAR (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN,  ##args)        ///< 警告
#define ERR (args...)   log_write(&g_log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, ##args)        ///< 错误
#endif


#ifndef bool
#define bool  unsigned char                 ///< 布尔类型
#endif

#ifndef true
#define true  1                             ///< 真值
#endif

#ifndef false
#define false 0                             ///< 假值
#endif

#ifndef NULL
#define NULL 0                              ///< 空
#endif

#ifndef SIZEOF
#define SIZEOF(x)   sizeof(x)/sizeof(x[0])  ///< 元素数量
#endif

/// 日志级别
typedef enum _LOG_LEVEL
{
    LOG_LEVEL_DEBUG,                ///< 调试
    LOG_LEVEL_INFO,                 ///< 信息
    LOG_LEVEL_WARN,                 ///< 警告
    LOG_LEVEL_ERROR                 ///< 错误

} LOG_LEVEL;

/// 日志文件保留周期
typedef enum _LOG_CYCLE
{
    LOG_CYCLE_MINUTE,               ///< 分钟
    LOG_CYCLE_HOUR,                 ///< 时
    LOG_CYCLE_DAY,                  ///< 天
    LOG_CYCLE_WEEK,                 ///< 周

} LOG_CYCLE;

/// 日志信息
typedef struct _xt_log
{
    char            filename[512];  ///< 日志文件名
    LOG_LEVEL       level;          ///< 日志级别(调试,信息,警告,错误)
    LOG_CYCLE       cycle;          ///< 日志文件保留周期(时,天,周)
    unsigned int    backup;         ///< 日志文件保留数量
    bool            clean;          ///< 首次打开日志文件时是否清空文件内容

    unsigned int    root;           ///< 文件目录根位置
    bool            run;            ///< 日志线程是否运行
    FILE*           file;           ///< 日志文件句柄

} xt_log, *p_xt_log;                ///< 日志信息指针

extern xt_log g_log;                ///< 全局日志指针

/**
 *\brief        初始化日志
 *\param[in]    filename    日志文件名前缀
 *\param[in]    level       日志级别(调试,信息,警告,错误)
 *\param[in]    cycle       日志文件保留周期(时,天,周)
 *\param[in]    backup      日志文件保留数量
 *\param[in]    clean       首次打开日志文件时是否清空文件内容
 *\param[in]    root        文件目录根位置
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init(const char *filename, LOG_LEVEL level, LOG_CYCLE cycle, unsigned int backup, bool clean, unsigned int root, p_xt_log log);

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
