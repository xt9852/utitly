/**
 *\file     xt_log.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2016.12.07
 *\brief    日志模块实现
 */
#include <pthread.h>
#include "xt_log.h"
#include "xt_utitly.h"

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
void log_get_filename(p_xt_log log, time_t timestamp, char *filename, int max)
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
int log_add_new(p_xt_log log, time_t timestamp, bool clean)
{
    if (NULL != log->file)
    {
        fclose(log->file);
    }

    char filename[LOG_FILENAME_SIZE];

    log_get_filename(log, timestamp, filename, LOG_FILENAME_SIZE);

    return fopen_s(&(log->file), filename, clean ? "wb+" : "ab+");
}

/**
 *\brief                    删除旧日志文件
 *\param[in]    log         日志数据
 *\param[in]    timestamp   时间戳
 *\return                   无
 */
void log_del_old(p_xt_log log, time_t timestamp)
{
    char filename[LOG_FILENAME_SIZE];

    log_get_filename(log, timestamp, filename, LOG_FILENAME_SIZE);

    _unlink(filename);      // 删除旧文件

    DD(log, "unlink %s\n", filename);
}

/**
 *\brief                    删除过期日志文件,查找文件创建时间小于删除时间的文件
 *\param[in]    log         日志数据
 *\return                   无
 */
void log_del_file(p_xt_log log)
{
    char                fmt[LOG_FILENAME_SIZE];
    struct tm           file_time;
    unsigned int        file_second;
    unsigned int        unit[] = { 60,  3600, 86400, 604800 };
    unsigned int        del_second = (unsigned int)time(NULL) - log->backup * unit[log->cycle];
    WIN32_FIND_DATAA    find;

    DD(log, "del_second:%u\n", del_second);

    snprintf(fmt, LOG_FILENAME_SIZE, "%s\\%s.????????????.txt", log->path, log->filename);
    DD(log, "find fmt:%s\n", fmt);

    HANDLE handle = FindFirstFileA(fmt, &find);

    if (INVALID_HANDLE_VALUE == handle)
    {
        EE(log, "FindFirstFileA error!\n");
        return;
    }

    snprintf(fmt, LOG_FILENAME_SIZE, "%s.%%04d%%02d%%02d%%02d%%02d.txt", log->filename);
    DD(log, "sscanf fmt:%s\n", fmt);

    do
    {
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { continue; }

        memset(&file_time, 0, sizeof(file_time));
        sscanf_s(find.cFileName, fmt, &file_time.tm_year, &file_time.tm_mon, &file_time.tm_mday, &file_time.tm_hour, &file_time.tm_min);

        file_time.tm_mon -= 1;      // 月(0-11)
        file_time.tm_year -= 1900;  // 自1900年起
        file_second = (unsigned int)mktime(&file_time);

        DD(log, "%s second:%u\n", find.cFileName, file_second);

        if (file_second <= del_second)
        {
            char tmp[LOG_FILENAME_SIZE];
            snprintf(tmp, LOG_FILENAME_SIZE, "%s\\%s", log->path, find.cFileName);
            _unlink(tmp);
            DD(log, "unlink %s\n", tmp);
        }
    }
    while(FindNextFileA(handle, &find));
}

/**
 *\brief                    日志后台线程
 *\return                   空
 */
void* log_thread(p_xt_log log)
{
    DD(log, "begin\n");

    bool reopen;
    unsigned int second = 0;
    unsigned int now_second;
    unsigned int del_second;
    unsigned int zone[] = {  0,    0, 28800,  28800 };
    unsigned int unit[] = { 60, 3600, 86400, 604800 };

    while (log->run)
    {
        msleep(100); // 100ms

        now_second = (unsigned int)time(NULL);

        if (now_second == second) { continue; } // 1秒之内

        second = now_second;

        DD(log, "now:%u\n", now_second);

        reopen = (((now_second + zone[log->cycle]) % unit[log->cycle]) == 0);

        if (!reopen) { continue; } // 创建新的文件

        del_second = now_second - log->backup * unit[log->cycle];

        DD(log, "del:%u\n", now_second);

        log_add_new(log, now_second, false);
        log_del_old(log, del_second);
    }

    DD(log, "exit\n");
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
    int ret = log_add_new(log, (int)time(NULL), log->clr_log);

    if (0 != ret)
    {
        printf("%s|open file:%s error:%d ret:%d\n", __FUNCTION__, log->filename, errno, ret);
        return -2;
    }

    DD(log, "日志格式:时时分分秒秒毫秒|进程ID日志级别(DIWE)线程ID|源文件:行号|函数名称|日志内容\n");

    if (0 == log->backup)   // 不删除旧文件,就不需要创建线程
    {
        DD(log, "backup:%d\n", log->backup);
        return 0;
    }

    log_del_file(log);      // 删除过期文件

    pthread_t tid;

    ret = pthread_create(&tid, NULL, log_thread, log);

    if (ret != 0)
    {
        EE(log, "create thread fail, error:%d\n", ret);
        return -3;
    }

    pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收

    g_xt_log = log;
    log->run = true;
    return 0;
}

/**
 *\brief                    初始化日志
 *\param[in]    path        日志文件路径
 *\param[in]    filename    日志文件名前缀
 *\param[in]    level       日志级别(调试,信息,警告,错误)
 *\param[in]    cycle       日志文件保留周期(时,天,周)
 *\param[in]    backup      日志文件保留数量
 *\param[in]    clr_log     首次打开日志文件时是否清空文件内容
 *\param[in]    del_old     首次打开日志文件时是否删除已经过期文件
 *\param[in]    root_len    代码根目录长度,日志中只保留相对目录
 *\param[out]   log         日志数据,需要filename,level,cycle,backup,clean
 *\attention    log         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int log_init_ex(const char *path, const char *filename, LOG_LEVEL level, LOG_CYCLE cycle, unsigned int backup,
                bool clr_log, bool del_old, unsigned int root_len, p_xt_log log)
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
    log->clr_log     = clr_log;
    log->del_old     = del_old;
    log->root_len    = root_len;
    log->run         = false;


    if (LOG_LEVEL_ERROR > LOG_CYCLE_WEEK)
    {
        printf("%s|param level error\n", __FUNCTION__);
        return -2;
    }

    if (cycle > LOG_CYCLE_WEEK)
    {
        printf("%s|param cycle error\n", __FUNCTION__);
        return -3;
    }

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
        DD(log, "log is null\n");
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

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec;
    localtime_s(&tm, &ts);

    len = snprintf(buf, LOG_BUFF_SIZE, "%02d%02d%02d%03d|%d%c%d|%s:%d|%s|",
                   tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
                   getpid(), XT_LOG_LEVEL[level], gettid(),
                   file + log->root_len, line, func);

    va_list arg;
    va_start(arg, fmt);

    len += vsnprintf(&buf[len], LOG_BUFF_SIZE - len, fmt, arg);

    va_end(arg);

    if (len >= LOG_BUFF_SIZE)
    {
        len = (int)strlen(buf); // 当buff不够时,vsnprintf返回的是需要的长度
    }

    fwrite(buf, 1, len, log->file); // 加1为多了个\n
    fflush(log->file);


}
