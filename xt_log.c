/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_log.c
 *\author       xt
 *\version      1.0.0
 *\date         2016.12.07
 *\brief        日志模块实现,UTF-8(No BOM)
 */
#include "xt_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
//#include <errno.h>
#include <pthread.h>

#ifdef _WINDOWS
#include <time.h>
#include <stdint.h>
#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, void *tz)
{
    FILETIME ft;
    uint64_t tmp = 0;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);

        tmp = ft.dwHighDateTime;
        tmp <<= 32;
        tmp |= ft.dwLowDateTime;
        tmp /= 10;  /*convert into microseconds*/
        tmp -= DELTA_EPOCH_IN_MICROSECS;

        tv->tv_sec = (long)(tmp / 1000000UL);
        tv->tv_usec = (long)(tmp % 1000000UL);
    }

    return 0;
}

#else
#include <sys/time.h>
#endif

#define BUFF_SIZE   10240                   ///< 日志缓冲区在小

const static char XT_LOG_LEVEL[] = "DIWE";  ///< 日志级别字符

/**
 *\brief        设置日志文件名
 *\param[in]    log         日志数据
 *\param[in]    timestamp   时间戳
 *\param[out]   filename    文件名
 *\param[in]    max         文件名最长
 *\return                   无
 */
void log_set_filename(p_xt_log log, time_t timestamp, char *filename, int max)
{
    struct tm tm;
    localtime_s(&tm, &timestamp);

    switch (log->cycle)
    {
        case LOG_CYCLE_MINUTE:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d%02d.txt", log->filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            break;
        }
        case LOG_CYCLE_HOUR:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d.txt", log->filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
            break;
        }
        default:
        {
            snprintf(filename, max, "%s.%d%02d%02d.txt", log->filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
    }
}

/**
 *\brief        新建日志文件
 *\param[in]    log         日志数据
 *\param[in]    timestamp   时间戳
 *\param[in]    clean       是否清空日志文件
 *\return       0           成功
 */
int log_add_new(p_xt_log log, int timestamp, bool clean)
{
    char filename[512];

    log_set_filename(log, timestamp, filename, sizeof(filename));

    if (NULL != log->file)
    {
        fclose(log->file);
    }

    return fopen_s(&(log->file), filename, clean ? "wb+" : "ab+");
}

/**
 *\brief        删除旧日志文件
 *\param[in]    log         日志数据
 *\param[in]    timestamp   时间戳
 *\return                   无
 */
void log_del_old(p_xt_log log, int timestamp)
{
    char filename[512] = "";

    log_set_filename(log, timestamp, filename, sizeof(filename));

    _unlink(filename);      // 删除旧文件

    log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "unlink %s", filename);
}

/**
 *\brief        日志后台线程
 *\return       空
 */
void* log_thread(p_xt_log log)
{
    log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "begin");

    bool reopen;
    unsigned int now;
    unsigned int del;
    unsigned int sec = 0;

    while (log->run)
    {
        msleep(100);

        now = (unsigned int)time(NULL);

        if (now == sec)
        {
            continue;
        }

        sec = now;

        log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "%u", now);

        switch (log->cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                reopen = ((now % 60) == 0);
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                reopen = ((now % 3600) == 0);
                break;
            }
            case LOG_CYCLE_DAY:
            {
                reopen = ((now % 86400) == 0);
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                reopen = ((now % 604800) == 0);
                break;
            }
        }

        if (!reopen)
        {
            continue;
        }

        switch (log->cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                del = now - log->backup * 60;
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                del = now - log->backup * 3600;
                break;
            }
            case LOG_CYCLE_DAY:
            {
                del = now - log->backup * 86400;
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                del = now - log->backup * 604800;
                break;
            }
        }

        log_add_new(log, now, false);
        log_del_old(log, del);
    }

    log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "exit");
    return NULL;
}

/**
 *\brief        初始化日志
 *\param[in]    log         日志数据,需要filename,level,cycle,backup,clean
 *\return       0           成功
 */
int log_init(p_xt_log log)
{
    if (NULL == log)
    {
        printf("%s|param is null\n", __FUNCTION__);
        return -1;
    }

    if (log->run)
    {
        printf("%s|inited\n", __FUNCTION__);
        return -2;
    }

    log->file = NULL;

    int ret = log_add_new(log, (int)time(NULL), log->clean);

    if (0 != ret)
    {
        return -3;
    }

    if (0 == log->backup)    // 不删除旧文件,就不需要创建线程
    {
        log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "backup:%d", log->backup);
        return 0;
    }

    log->run = true;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    // 退出时自行释放所占用的资源

    ret = pthread_create(&tid, &attr, log_thread, log);

    if (ret != 0)
    {
        log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "create thread fail, err:%d", ret);
        return -3;
    }

    log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "---------------------------------------------------ok");
    return 0;
}

/**
 *\brief        反初始化日志
 *\param[in]    log         日志数据
 *\return       无
 */
void log_uninit(p_xt_log log)
{
    if (NULL == log || NULL == log->file)
    {
        log_write(log, __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, "log is null");
        return;
    }

    log->run = false;
    fflush(log->file);
    fclose(log->file);
    log->file = NULL;
}

/**
 *\brief        写日志不带锁
 *\param[in]    log         日志数据
 *\param[in]    file        文件名
 *\param[in]    func        函数名
 *\param[in]    line        行号
 *\param[in]    level       日志级别
 *\param[in]    fmt         日志内容
 *\return                   无
 */
void log_write(p_xt_log log, const char *file, const char *func, int line, int level, const char *fmt, ...)
{
    if (NULL == log || level < log->level)
    {
        return;
    }

    int             len;
    char            buff[BUFF_SIZE];
    time_t          ts;
    struct tm       tm;
    struct timeval  tv;

    va_list arg;
    va_start(arg, fmt);

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec;
    localtime_s(&tm, &ts);

    len = snprintf(buff, BUFF_SIZE, "%02d:%02d:%02d.%03d|%d|%d|%c|%s:%d|%s|",
                   tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
                   getpid(), gettid(), XT_LOG_LEVEL[level], file, line, func);

    len += vsnprintf(&buff[len], BUFF_SIZE - len, fmt, arg);

    if (len >= BUFF_SIZE)
    {
        len = (int)strlen(buff); // 当buff不够时,vsnprintf返回的是需要的长度
    }

    buff[len] = '\n';

    fwrite(buff, 1, len + 1, log->file); // 加1为多了个\n
    fflush(log->file);

    va_end(arg);
}
