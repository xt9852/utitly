/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_log.c
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 日志模块实现
*************************************************/

#include "xt_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "cJSON.h"

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


enum
{
    LOG_CYCLE_MINUTE,
    LOG_CYCLE_HOUR,
    LOG_CYCLE_DAY,
    LOG_CYCLE_WEEK,
};

typedef struct _log_info
{
    char name[512];         // 日志文件名
    int  level;             // 日志级别(调试,信息,警告,错误)
    int  cycle;             // 日志文件保留周期(时,天,周)
    int  backup;            // 日志文件保留数量
    bool load;              // 是否已经加载数据
    bool init;              // 是否已经初始化

}log_info, *p_log_info;

#define BUFF_SIZE           10240

static char                 LEVEL[] = "DIWE";
static FILE                *g_log = NULL;
static log_info             g_info = {0};
static pthread_mutex_t      g_mutex;

/**
 *\brief      设置日志文件名
 *\param[in]  int           timestamp   时间戳
 *\param[out] char         *filename    文件名
 *\param[in]  int           max         文件名最长
 *\return     无
 */
void xt_log_set_filename(time_t timestamp, char *filename, int max)
{
    struct tm tm;
    localtime_s(&tm, &timestamp);

    switch (g_info.cycle)
    {
        case LOG_CYCLE_MINUTE:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d%02d.txt", g_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            break;
        }
        case LOG_CYCLE_HOUR:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d.txt", g_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
            break;
        }
        default:
        {
            snprintf(filename, max, "%s.%d%02d%02d.txt", g_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
    }
}

/**
 *\brief      新建日志文件
 *\param[in]  int           timestamp   时间戳
 *\return     无
 */
void xt_log_add_new(int timestamp)
{
    char filename[512];
    xt_log_set_filename(timestamp, filename, sizeof(filename));
    freopen_s(&g_log, filename, "ab+", stderr); // 使用同一个句柄打开此文件
}

/**
 *\brief      删除旧日志文件
 *\param[in]  int           timestamp   时间戳
 *\return     无
 */
void xt_log_del_old(int timestamp)
{
    char filename[512] = "";
    xt_log_set_filename(timestamp, filename, sizeof(filename));
    _unlink(filename);                          // 删除旧文件
    DBG("unlink %s", filename);
}

/**
 *\brief      日志后台线程
 *\return     空
 */
void *xt_log_thread(void *arg)
{
    bool proc;
    int time_add;
    int time_del;

    while (true)
    {
        sleep(1);

        time_add = (int)time(NULL);
        
        DBG("%d", time_add);

        switch (g_info.cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                proc = ((time_add % 60) == 0);
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                proc = ((time_add % 3600) == 0);
                break;
            }
            case LOG_CYCLE_DAY:
            {
                proc = ((time_add % 86400) == 0);
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                proc = ((time_add % 604800) == 0);
                break;
            }
        }

        if (!proc)
        {
            continue;
        }

        switch (g_info.cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                time_del = time_add - g_info.backup * 60;
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                time_del = time_add - g_info.backup * 3600;
                break;
            }
            case LOG_CYCLE_DAY:
            {
                time_del = time_add - g_info.backup * 86400;
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                time_del = time_add - g_info.backup * 604800;
                break;
            }
        }

        xt_log_add_new(time_add);

        xt_log_del_old(time_del);
    }

    return NULL;
}

/**
 *\brief      加载配置文件
 *\param[in]  const char    *path       日志文件所在目录
 *\param[in]  void          *json       配置文件json根
 *\return     0-成功
 */
int xt_log_parse_config(const char *path, void *json)
{
    if (NULL == path || NULL == json)
    {
        printf("%s|param is null\n", __FUNCTION__);
        return -1;
    }

    cJSON *root = (cJSON*)json;

    cJSON *name = cJSON_GetObjectItem(root, "name");

    if (NULL == name)
    {
        printf("%s|config json no log.name node\n", __FUNCTION__);
        return -2;
    }

    snprintf(g_info.name, sizeof(g_info.name), "%s%s", path, name->valuestring);

    cJSON *level = cJSON_GetObjectItem(root, "level");

    if (NULL == level)
    {
        printf("%s|config json no log.level node\n", __FUNCTION__);
        return -3;
    }

    if (0 == strcmp(level->valuestring, "debug"))
    {
        g_info.level = LOG_LEVEL_DEBUG;
    }
    else if (0 == strcmp(level->valuestring, "info"))
    {
        g_info.level = LOG_LEVEL_INFO;
    }
    else if (0 == strcmp(level->valuestring, "warn"))
    {
        g_info.level = LOG_LEVEL_WARN;
    }
    else if (0 == strcmp(level->valuestring, "error"))
    {
        g_info.level = LOG_LEVEL_ERROR;
    }
    else
    {
        printf("%s|config json no log.level value error\n", __FUNCTION__);
        return -4;
    }

    cJSON *cycle = cJSON_GetObjectItem(root, "cycle");

    if (NULL == cycle)
    {
        printf("%s|config json no log.cycle node\n", __FUNCTION__);
        return -5;
    }

    if (0 == strcmp(cycle->valuestring, "minute"))
    {
        g_info.cycle = LOG_CYCLE_MINUTE;
    }
    else if (0 == strcmp(cycle->valuestring, "hour"))
    {
        g_info.cycle = LOG_CYCLE_HOUR;
    }
    else if (0 == strcmp(cycle->valuestring, "day"))
    {
        g_info.cycle = LOG_CYCLE_DAY;
    }
    else if (0 == strcmp(cycle->valuestring, "week"))
    {
        g_info.cycle = LOG_CYCLE_WEEK;
    }
    else
    {
        printf("%s|config no log.cycle value error\n", __FUNCTION__);
        return -6;
    }

    cJSON *backup = cJSON_GetObjectItem(root, "backup");

    if (NULL == backup)
    {
        printf("%s|config no log.backup value error\n", __FUNCTION__);
        return -7;
    }

    g_info.backup = backup->valueint;

    g_info.load = true;

    return 0;
}

/**
 *\brief      初始化日志
 *\param[in]  p_log_info info   信息
 *\return     int 0-成功,~0-失败
 */
int xt_log_init()
{
    if (!g_info.load)
    {
        printf("%s|data is not load\n", __FUNCTION__);
        return -1;
    }

    if (g_info.init)
    {
        printf("%s|inited\n", __FUNCTION__);
        return -2;
    }

    pthread_mutex_init(&g_mutex, NULL);

    xt_log_add_new((int)time(NULL));

    if (NULL == g_log)
    {
        return -3;
    }

    g_info.init = true;

    if (g_info.backup <= 0)
    {
        DBG("backup:%d", g_info.backup);
        return 0;
    }

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, xt_log_thread, NULL);

    if (ret != 0)
    {
        ERR("create thread fail, err:%d\n", ret);
        return -3;
    }

    DBG("---------------------------------------------------");
    DBG("ok");
    return 0;
}

/**
 *\brief      反初始化日志
 *\return     无
 */
void xt_log_uninit()
{
    if (g_info.init && NULL != g_log)
    {
        fflush(g_log);
        fclose(g_log);
        g_log = NULL;
    }

    pthread_mutex_destroy(&g_mutex);
}

/**
 *\brief      写日志
 *\param[in]  const char *file  文件名
 *\param[in]  const char *func  函数名
 *\param[in]  int line          行号
 *\param[in]  int level         日志级别
 *\param[in]  const char *fmt   日志内容
 *\return     无
 */
void xt_log(const char *file, const char *func, int line, int level, const char *fmt, va_list arg)
{
    int len;
    char buff[BUFF_SIZE];
    time_t sec;
    struct tm tm;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    localtime_s(&tm, &sec);

    len = snprintf(buff, BUFF_SIZE, "%02d:%02d:%02d.%03d|%d|%d|%c|%s:%d|%s|",
                   tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
                   getpid(), gettid(), LEVEL[level], file, line, func);

    len += vsnprintf(&buff[len], BUFF_SIZE - len, fmt, arg);

    if (len >= BUFF_SIZE)
    {
        len = (int)strlen(buff); // 当buff不够时,vsnprintf返回的是需要的长度
    }

    buff[len] = '\n';
    fwrite(buff, 1, len + 1, g_log); // 加1为多了个\n
    fflush(g_log);
}

/**
 *\brief      写日志不带锁
 *\param[in]  const char *file  文件名
 *\param[in]  const char *func  函数名
 *\param[in]  int line          行号
 *\param[in]  int level         日志级别
 *\param[in]  const char *fmt   日志内容
 *\return     无
 */
void xt_log_write(const char *file, const char *func, int line, int level, const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);

    xt_log(file, func, line, level, fmt, arg);

    va_end(arg);
}

/**
 *\brief      写日志带锁
 *\param[in]  const char *file  文件名
 *\param[in]  const char *func  函数名
 *\param[in]  int line          行号
 *\param[in]  int level         日志级别
 *\param[in]  const char *fmt   日志内容
 *\return     无
 */
void xt_log_write_lock(const char *file, const char *func, int line, int level, const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);

    pthread_mutex_lock(&g_mutex);

    xt_log(file, func, line, level, fmt, arg);

    pthread_mutex_unlock(&g_mutex);

    va_end(arg);
}