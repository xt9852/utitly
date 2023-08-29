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
#include <stdio.h>  // FILE
#include "xt_utitly.h"

#define FFL                 __FILE__, __FUNCTION__, __LINE__                ///< 源文件,函数名称,行号
#define LOG_FILENAME_SIZE   512                                             ///< 日志文件名缓冲区大小

#ifdef _WINDOWS
    #define DD(l, ...)  log_write(l,        FFL, LOG_LEVEL_DEBUG, __VA_ARGS__)  ///< 调试,指定文件输出
    #define II(l, ...)  log_write(l,        FFL, LOG_LEVEL_INFO,  __VA_ARGS__)  ///< 信息,指定文件输出
    #define WW(l, ...)  log_write(l,        FFL, LOG_LEVEL_WARN,  __VA_ARGS__)  ///< 警告,指定文件输出
    #define EE(l, ...)  log_write(l,        FFL, LOG_LEVEL_ERROR, __VA_ARGS__)  ///< 错误,指定文件输出
    #define D(...)      log_write(g_xt_log, FFL, LOG_LEVEL_DEBUG, __VA_ARGS__)  ///< 调试
    #define I(...)      log_write(g_xt_log, FFL, LOG_LEVEL_INFO,  __VA_ARGS__)  ///< 信息
    #define W(...)      log_write(g_xt_log, FFL, LOG_LEVEL_WARN,  __VA_ARGS__)  ///< 警告
    #define E(...)      log_write(g_xt_log, FFL, LOG_LEVEL_ERROR, __VA_ARGS__)  ///< 错误
#else
    #define D (args...) log_write(g_xt_log, FFL, LOG_LEVEL_DEBUG, ##args)       ///< 调试
    #define I (args...) log_write(g_xt_log, FFL, LOG_LEVEL_INFO,  ##args)       ///< 信息
    #define W (args...) log_write(g_xt_log, FFL, LOG_LEVEL_WARN,  ##args)       ///< 警告
    #define E (args...) log_write(g_xt_log, FFL, LOG_LEVEL_ERROR, ##args)       ///< 错误
#endif


/// 日志级别
typedef enum _LOG_LEVEL
{
    LOG_LEVEL_DEBUG,                                ///< 调试
    LOG_LEVEL_INFO,                                 ///< 信息
    LOG_LEVEL_WARN,                                 ///< 警告
    LOG_LEVEL_ERROR                                 ///< 错误

} LOG_LEVEL;

/// 日志文件保留周期
typedef enum _LOG_CYCLE
{
    LOG_CYCLE_MINUTE,                               ///< 分钟
    LOG_CYCLE_HOUR,                                 ///< 时
    LOG_CYCLE_DAY,                                  ///< 天
    LOG_CYCLE_WEEK,                                 ///< 周

} LOG_CYCLE;

/// 日志信息
typedef struct _xt_log
{
    char            path[LOG_FILENAME_SIZE];        ///< 日志文件路径
    char            filename[LOG_FILENAME_SIZE];    ///< 日志文件名
    LOG_LEVEL       level;                          ///< 日志级别(调试,信息,警告,错误)
    LOG_CYCLE       cycle;                          ///< 日志文件保留周期(时,天,周)
    unsigned int    backup;                         ///< 日志文件保留数量
    bool            clean_log;                      ///< 首次打开日志文件时是否清空文件内容
    bool            clean_file;                     ///< 首次打开日志文件时是否删除过期文件内容


    unsigned int    root;                           ///< 代码根目录长度,日志中只保留相对目录
    bool            run;                            ///< 日志线程是否运行
    FILE*           file;                           ///< 日志文件句柄

} xt_log, *p_xt_log;                                ///< 日志信息指针

p_xt_log            g_xt_log;                       ///< 全局日志指针

/**
 *\brief        初始化日志
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init(p_xt_log log);

/**
 *\brief        初始化日志
 *\param[in]    path        日志文件路径
 *\param[in]    filename    日志文件名前缀
 *\param[in]    level       日志级别(调试,信息,警告,错误)
 *\param[in]    cycle       日志文件保留周期(时,天,周)
 *\param[in]    backup      日志文件保留数量
 *\param[in]    clean_log   首次打开日志文件时是否清空文件内容
 *\param[in]    clean_file  首次打开日志文件时是否删除已经过期文件
 *\param[in]    root        代码根目录长度,日志中只保留相对目录
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init_ex(const char *path, const char *filename, LOG_LEVEL level, LOG_CYCLE cycle, unsigned int backup, bool clean_log, bool clean_file, unsigned int root, p_xt_log log);

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
