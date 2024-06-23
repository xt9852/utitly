/**
 *\file     xt_log.c
 *\author   xt
 *\version  1.0.0
 *\date     2016.12.07
 *\brief    日志模块实现
 */
#include "xt_log.h"
#include "xt_utitly.h"
#include <pthread.h>

#define LOG_BUFF_SIZE   10240               ///< 日志缓冲区在小

const static char XT_LOG_LEVEL[] = "DIWE";  ///< 日志级别字符

/**
 *\brief                    设置日志文件名
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
 *\brief                    新建日志文件
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
 *\brief                    删除旧日志文件
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
 *\brief                    删除过期日志文件
 *\param[in]    log         日志数据
 *\return                   无
 */
void log_del_file(p_xt_log log)
{
    int cnt;
    char *mask;
    unsigned int del;
    unsigned int now = (unsigned int)time(NULL);

    switch (log->cycle)
    {
        case LOG_CYCLE_MINUTE:
        {
            cnt = 12;
            mask = "????????????";
            del = now - log->backup * 60;
            break;
        }
        case LOG_CYCLE_HOUR:
        {
            cnt = 10;
            mask = "??????????";
            del = now - log->backup * 3600;
            break;
        }
        case LOG_CYCLE_DAY:
        {
            cnt = 8;
            mask = "????????";
            del = now - log->backup * 86400;
            break;
        }
        case LOG_CYCLE_WEEK:
        {
            cnt = 8;
            mask = "????????";
            del = now - log->backup * 604800;
            break;
        }
        default:
        {
            return;
        }
    }

    // 查找文件删除时间小于del的文件
    char        *end;
    char         tmp[LOG_FILENAME_SIZE];
    unsigned int sec;

    struct tm t;
    WIN32_FIND_DATAA data;

    snprintf(tmp, LOG_FILENAME_SIZE, "%s\\%s.%s.txt", log->path, log->filename, mask);

    HANDLE err = FindFirstFileA(tmp, &data);

    if (err == INVALID_HANDLE_VALUE)
    {
        printf("FindFirstFileA error!");
        return;
    }

    do
    {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        end = strrchr(data.cFileName, '.');
        strncpy_s(tmp, LOG_FILENAME_SIZE, end - cnt, cnt); // 年月日时间字符串
        memset(&t, 0, sizeof(t));

        switch (log->cycle)
        {
            case LOG_CYCLE_MINUTE:
            {
                sscanf_s(tmp, "%04d%02d%02d%02d%02d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min);
                break;
            }
            case LOG_CYCLE_HOUR:
            {
                sscanf_s(tmp, "%04d%02d%02d%02d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour);
                break;
            }
            default:
            {
                sscanf_s(tmp, "%04d%02d%02d", &t.tm_year, &t.tm_mon, &t.tm_mday);
                break;
            }
        }

        t.tm_mon -= 1;      // 月(0-11)
        t.tm_year -= 1900;  // 自1900年起
        sec = (unsigned int)mktime(&t);

        DD(log, "%s %s %u %u", data.cFileName, tmp, sec, del);

        if (sec <= del)
        {
            snprintf(tmp, LOG_FILENAME_SIZE, "%s\\%s", log->path, data.cFileName);
            _unlink(tmp);
            DD(log, "unlink %s", tmp);
        }
    }
    while(FindNextFileA(err, &data));
}

/**
 *\brief                    日志后台线程
 *\return                   空
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
 *\brief                    初始化日志
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean_log,clean_file
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init(p_xt_log log)
{
    int ret = log_add_new(log, (int)time(NULL), log->clean_log);

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

    ret = pthread_create(&tid, NULL, log_thread, log);

    if (ret != 0)
    {
        EE(log, "create thread fail, error:%d", ret);
        return -3;
    }

    pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收

    DD(log, "日志格式:时时分分秒秒毫秒|进程ID日志级别(DIWE)线程ID|源文件:行号|函数名称|日志内容");

    log_del_file(log);      // 删除过期文件
    return 0;
}

/**
 *\brief                    初始化日志
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
int log_init_ex(const char *path, const char *filename, LOG_LEVEL level, LOG_CYCLE cycle, unsigned int backup, bool clean_log, bool clean_file, unsigned int root, p_xt_log log)
{
    if (NULL == filename || NULL == log)
    {
        printf("%s|param is null\n", __FUNCTION__);
        return -1;
    }

    strncpy_s(log->path, LOG_FILENAME_SIZE, path, LOG_FILENAME_SIZE - 1);
    strncpy_s(log->filename, LOG_FILENAME_SIZE, filename, LOG_FILENAME_SIZE - 1);
    log->file        = NULL;
    log->level       = level;
    log->cycle       = cycle;
    log->backup      = backup;
    log->clean_log   = clean_log;
    log->clean_file  = clean_file;
    log->root        = root;
    log->run         = false;

    return log_init(log);
}

/**
 *\brief                    反初始化日志
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
 *\brief                    写日志不带锁
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
