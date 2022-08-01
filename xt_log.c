/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_log.c
 *\author       xt
 *\version      1.0.0
 *\date         2016.12.07
 *\brief        日志模块实现,UTF-8(No BOM)
 */
#include "xt_log.h"
#include "xt_utitly.h"
#include <pthread.h>

#define LOG_BUFF_SIZE   10240               ///< 日志缓冲区在小

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
            snprintf(filename, max, "%s.%d%02d%02d%02d%02d.txt", log->filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            break;
        }
        case LOG_CYCLE_HOUR:
        {
            snprintf(filename, max, "%s.%d%02d%02d%02d.txt", log->filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
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
    char filename[LOG_FILENAME_SIZE];

    log_set_filename(log, timestamp, filename, LOG_FILENAME_SIZE);

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
    char filename[LOG_FILENAME_SIZE] = "";

    log_set_filename(log, timestamp, filename, LOG_FILENAME_SIZE);

    _unlink(filename);      // 删除旧文件

    DD(log, "unlink %s", filename);
}

/**
 *\brief        日志后台线程
 *\return       空
 */
void* log_thread(p_xt_log log)
{
    DD(log, "begin");

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

        //DD(log, "%u", now);

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
                reopen = (((now + 28800) % 86400) == 0);
                break;
            }
            case LOG_CYCLE_WEEK:
            {
                reopen = (((now + 28800) % 604800) == 0);
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

    DD(log, "exit");
    return NULL;
}

/**
 *\brief        初始化日志
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init(p_xt_log log)
{
    int ret = log_add_new(log, (int)time(NULL), log->clean);

    if (0 != ret)
    {
        printf("%s|open file:%s error:%d\n", __FUNCTION__, log->filename, errno);
        return -2;
    }

    if (0 == log->backup)   // 不删除旧文件,就不需要创建线程
    {
        DD(log, "backup:%d", log->backup);
        return 0;
    }

    g_xt_log = log;

    log->run = true;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    // 退出时自行释放所占用的资源

    ret = pthread_create(&tid, &attr, log_thread, log);

    if (ret != 0)
    {
        EE(log, "create thread fail, error:%d", ret);
        return -3;
    }

    DD(log, "格式:时时分分秒秒毫秒|进程ID日志级别(DIWE)线程ID|源文件:行号|函数名称|日志内容");
    return 0;
}

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
int log_init_ex(const char *filename, LOG_LEVEL level, LOG_CYCLE cycle, unsigned int backup, bool clean, unsigned int root, p_xt_log log)
{
    if (NULL == filename || NULL == log)
    {
        printf("%s|param is null\n", __FUNCTION__);
        return -1;
    }

    strncpy_s(log->filename, LOG_FILENAME_SIZE, filename, LOG_FILENAME_SIZE - 1);
    log->file   = NULL;
    log->level  = level;
    log->cycle  = cycle;
    log->backup = backup;
    log->clean  = clean;
    log->root   = root;
    log->run    = false;

    return log_init(log);
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
        DD(log, "log is null");
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
    char            buf[LOG_BUFF_SIZE];
    time_t          ts;
    struct tm       tm;
    struct timeval  tv;

    va_list arg;
    va_start(arg, fmt);

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec;
    localtime_s(&tm, &ts);

    len = snprintf(buf, LOG_BUFF_SIZE, "%02d%02d%02d%03d|%d%c%d|%s:%d|%s|",
                   tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
                   getpid(), XT_LOG_LEVEL[level], gettid(),
                   file + log->root, line, func);

    len += vsnprintf(&buf[len], LOG_BUFF_SIZE - len, fmt, arg);

    if (len >= LOG_BUFF_SIZE)
    {
        len = (int)strlen(buf); // 当buff不够时,vsnprintf返回的是需要的长度
    }

    buf[len] = '\n';

    fwrite(buf, 1, len + 1, log->file); // 加1为多了个\n
    fflush(log->file);

    va_end(arg);
}
