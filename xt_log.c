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

/// 日志文件保留周期
enum
{
    LOG_CYCLE_MINUTE,                       ///< 分钟
    LOG_CYCLE_HOUR,                         ///< 小时
    LOG_CYCLE_DAY,                          ///< 天
    LOG_CYCLE_WEEK,                         ///< 周
};

/// 日志信息
typedef struct _log_info
{
    char            name[512];              ///< 日志文件名
    int             level;                  ///< 日志级别(调试,信息,警告,错误)
    int             cycle;                  ///< 日志文件保留周期(时,天,周)
    int             backup;                 ///< 日志文件保留数量
    bool            clean;                  ///< 首次打开日志文件时是否清空文件内容
    bool            init;                   ///< 是否已经初始化
    bool            run;                    ///< 日志线程是否运行
    FILE*           file;                   ///< 日志文件句柄

}log_info, *p_log_info;

#define BUFF_SIZE   10240                   ///< 日志缓冲区在小

static log_info     g_log_info  = {0};      ///< 日志信息

/**
 *\brief        设置日志文件名
 *\param[in]    timestamp   时间戳
 *\param[out]   filename    文件名
 *\param[in]    max         文件名最长
 *\return                   无
 */
void xt_log_set_filename(time_t timestamp, char *filename, int max)
{
    struct tm tm;
    localtime_s(&tm, &timestamp);

    switch (g_log_info.cycle)
    {
        case LOG_CYCLE_MINUTE:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d%02d.txt", g_log_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            break;
        }
        case LOG_CYCLE_HOUR:
        {
            snprintf(filename, max, "%s.%d%02d%02d-%02d.txt", g_log_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
            break;
        }
        default:
        {
            snprintf(filename, max, "%s.%d%02d%02d.txt", g_log_info.name, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
    }
}

/**
 *\brief        新建日志文件
 *\param[in]    timestamp   时间戳
 *\param[in]    clean       是否清空文件内容
 *\return                   无
 */
void xt_log_add_new(int timestamp, bool clean)
{
    char filename[512];
    xt_log_set_filename(timestamp, filename, sizeof(filename));
    freopen_s(&(g_log_info.file), filename, clean ? "wb+" : "ab+", stderr); // 使用同一个句柄打开此文件
}

/**
 *\brief        删除旧日志文件
 *\param[in]    timestamp   时间戳
 *\return                   无
 */
void xt_log_del_old(int timestamp)
{
    char filename[512] = "";
    xt_log_set_filename(timestamp, filename, sizeof(filename));
    _unlink(filename);                          // 删除旧文件
    DBG("unlink %s", filename);
}

/**
 *\brief        日志后台线程
 *\return       空
 */
void *xt_log_thread(void *param)
{
    DBG("begin");

    bool reopen;
    unsigned int now;
    unsigned int del;
    unsigned int sec = 0;

    while (g_log_info.run)
    {
        msleep(100);

        now = (unsigned int)time(NULL);

        if (now == sec)
        {
            continue;
        }

        sec = now;

        //DBG("%u", now);

        switch (g_log_info.cycle)
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

        switch (g_log_info.cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                del = now - g_log_info.backup * 60;
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                del = now - g_log_info.backup * 3600;
                break;
            }
            case LOG_CYCLE_DAY:
            {
                del = now - g_log_info.backup * 86400;
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                del = now - g_log_info.backup * 604800;
                break;
            }
        }

        xt_log_add_new(now, false);
        xt_log_del_old(del);
    }

    DBG("exit");
    return NULL;
}

/**
 *\brief        加载配置文件
 *\param[in]    path        日志文件所在目录
 *\param[in]    json        配置文件json根
 *\return       0           成功
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

    snprintf(g_log_info.name, sizeof(g_log_info.name), "%s%s", path, name->valuestring);

    cJSON *level = cJSON_GetObjectItem(root, "level");

    if (NULL == level)
    {
        printf("%s|config json no log.level node\n", __FUNCTION__);
        return -3;
    }

    if (0 == strcmp(level->valuestring, "debug"))
    {
        g_log_info.level = LOG_LEVEL_DEBUG;
    }
    else if (0 == strcmp(level->valuestring, "info"))
    {
        g_log_info.level = LOG_LEVEL_INFO;
    }
    else if (0 == strcmp(level->valuestring, "warn"))
    {
        g_log_info.level = LOG_LEVEL_WARN;
    }
    else if (0 == strcmp(level->valuestring, "error"))
    {
        g_log_info.level = LOG_LEVEL_ERROR;
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
        g_log_info.cycle = LOG_CYCLE_MINUTE;
    }
    else if (0 == strcmp(cycle->valuestring, "hour"))
    {
        g_log_info.cycle = LOG_CYCLE_HOUR;
    }
    else if (0 == strcmp(cycle->valuestring, "day"))
    {
        g_log_info.cycle = LOG_CYCLE_DAY;
    }
    else if (0 == strcmp(cycle->valuestring, "week"))
    {
        g_log_info.cycle = LOG_CYCLE_WEEK;
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

    g_log_info.backup = backup->valueint;

    cJSON *clean = cJSON_GetObjectItem(root, "clean");

    g_log_info.clean = (NULL != clean) ? clean->valueint : false;

    return 0;
}

/**
 *\brief        初始化日志
 *\param[in]    path    日志文件所在目录
 *\param[in]    json    配置文件json根
 *\return       0       成功
 */
int xt_log_init(const char *path, void *json)
{
    if (NULL == path || NULL == json)
    {
        printf("%s|param is null\n", __FUNCTION__);
        return -1;
    }

    if (g_log_info.init)
    {
        printf("%s|inited\n", __FUNCTION__);
        return -2;
    }

    int ret = xt_log_parse_config(path, json);

    if (0 != ret)
    {
        return ret;
    }

    xt_log_add_new((int)time(NULL), g_log_info.clean);

    if (NULL == g_log_info.file)
    {
        return -3;
    }

    if (g_log_info.backup <= 0)
    {
        DBG("backup:%d", g_log_info.backup);
        return 0;
    }

    g_log_info.run  = true;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    // 退出时自行释放所占用的资源

    ret = pthread_create(&tid, &attr, xt_log_thread, NULL);

    if (ret != 0)
    {
        ERR("create thread fail, err:%d\n", ret);
        return -3;
    }

    g_log_info.init = true;

    DBG("---------------------------------------------------ok");
    return 0;
}

/**
 *\brief        反初始化日志
 *\return       无
 */
void xt_log_uninit()
{
    g_log_info.run = false;

    if (NULL != g_log_info.file)
    {
        fflush(g_log_info.file);
        fclose(g_log_info.file);
        g_log_info.file = NULL;
    }

    g_log_info.init = false;
}

/**
 *\brief        写日志不带锁
 *\param[in]    file        文件名
 *\param[in]    func        函数名
 *\param[in]    line        行号
 *\param[in]    level       日志级别
 *\param[in]    fmt         日志内容
 *\return                   无
 */
void xt_log_write(const char *file, const char *func, int line, char level, const char *fmt, ...)
{
    int    len;
    char   buff[BUFF_SIZE];
    time_t         ts;
    struct tm      tm;
    struct timeval tv;

    va_list arg;
    va_start(arg, fmt);

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec;
    localtime_s(&tm, &ts);

    len = snprintf(buff, BUFF_SIZE, "%02d:%02d:%02d.%03d|%d|%d|%c|%s:%d|%s|",
                   tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
                   getpid(), gettid(), level, file, line, func);

    len += vsnprintf(&buff[len], BUFF_SIZE - len, fmt, arg);

    if (len >= BUFF_SIZE)
    {
        len = (int)strlen(buff); // 当buff不够时,vsnprintf返回的是需要的长度
    }

    buff[len] = '\n';

    fwrite(buff, 1, len + 1, g_log_info.file); // 加1为多了个\n
    fflush(g_log_info.file);

    va_end(arg);
}
