/**
 *\file     xt_timer.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2014.6.24
 *\brief    定时器模块器实现
 */
#include <pthread.h>
#include <time.h>
#include "xt_timer.h"
#include "xt_list.h"
#include "xt_utitly.h"

#ifdef XT_LOG
    #include "xt_log.h"
#else
    #include <stdio.h>
    #include <stdlib.h>
#ifdef _WINDOWS
    #define D(...)      printf(__VA_ARGS__)
    #define I(...)      printf(__VA_ARGS__)
    #define W(...)      printf(__VA_ARGS__)
    #define E(...)      printf(__VA_ARGS__)
#else
    #define D(args...)  printf(args)
    #define I(args...)  printf(args)
    #define W(args...)  printf(args)
    #define E(args...)  printf(args)
#endif
#endif

/**
 *\brief                    检查定时器
 *\param[in]    timer       定时器
 *\param[in]    param       自定义参数
 *\return       0
 */
int timer_check(p_xt_timer timer, void *param)
{
    unsigned int now = (unsigned int)param;

    if (TIMER_TYPE_CYCLE == timer->type)    // 按周期执行
    {
        if (now >= timer->cycle_next)
        {
            timer->cycle_next += timer->cycle_second;

            D("name:%s now:%u next:%u\n", timer->name, now, timer->cycle_next);

            thread_pool_put(timer->thread_pool, timer->task.proc, timer->task.param);
        }
    }
    else
    {
        // |   6    |    5    |    4   |    3    |    2    |    1   |    0   |
        // |tm_yday | tm_wday | tm_mon | tm_mday | tm_hour | tm_min | tm_sec |

        struct tm tm;
        time_t ts = now;
        localtime_s(&tm, &ts);

        unsigned int mask =
            ((tm.tm_yday == timer->cron_yday) << 6) |   // 注意是==等于
            ((tm.tm_wday == timer->cron_wday) << 5) |
            ((tm.tm_mon  == timer->cron_mon)  << 4) |
            ((tm.tm_mday == timer->cron_mday) << 3) |
            ((tm.tm_hour == timer->cron_hour) << 2) |
            ((tm.tm_min  == timer->cron_min)  << 1) |
            ((tm.tm_sec  == timer->cron_sec));

        if (((TIMER_CRON_YDAY   == timer->type) && ((mask&0x47) == 0x47)) ||    // 每年的第yday天的hour时min分sec秒执行任务
            ((TIMER_CRON_WDAY   == timer->type) && ((mask&0x27) == 0x27)) ||    // 每周的第wday天的hour时min分sec秒执行任务
            ((TIMER_CRON_YEAR   == timer->type) && ((mask&0x1F) == 0x1F)) ||    // 每年的第mon月第mday天的hour时min分sec秒执行任务
            ((TIMER_CRON_MON    == timer->type) && ((mask&0x0F) == 0x0F)) ||    // 每月的第mday天的hour时min分sec秒执行任务
            ((TIMER_CRON_DAY    == timer->type) && ((mask&0x07) == 0x07)) ||    // 每天的hour时min分sec秒执行任务
            ((TIMER_CRON_HOUR   == timer->type) && ((mask&0x03) == 0x03)) ||    // 每时的min分sec秒执行任务
            ((TIMER_CRON_MINUTE == timer->type) && ((mask&0x01) == 0x01)))      // 每分的sec秒执行任务
        {
            D("name:%s y:%d w:%d m:%d d:%d h:%d m:%d s:%d",
                 timer->name,
                 tm.tm_yday,
                 tm.tm_wday,
                 tm.tm_mon,
                 tm.tm_mday,
                 tm.tm_hour,
                 tm.tm_min,
                 tm.tm_sec);

            thread_pool_put(timer->thread_pool, timer->task.proc, timer->task.param);

            switch (timer->type)
            {
            case TIMER_CRON_YDAY:
                {
                    D("every year %d day %d:%d:%d : %s\n", timer->cron_yday, timer->cron_hour, timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            case TIMER_CRON_WDAY:
                {
                    D("every week %d day %d:%d:%d : %s\n", timer->cron_wday, timer->cron_hour, timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            case TIMER_CRON_YEAR:
                {
                    D("every year %d-%d %d:%d:%d : %s\n", timer->cron_mon, timer->cron_mday, timer->cron_hour, timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            case TIMER_CRON_MON:
                {
                    D("every month %d day %d:%d:%d : %s\n", timer->cron_mday, timer->cron_hour, timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            case TIMER_CRON_DAY:
                {
                    D("every day %d:%d:%d : %s\n", timer->cron_hour, timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            case TIMER_CRON_HOUR:
                {
                    D("%d:%d : %s\n", timer->cron_min, timer->cron_sec, timer->name);
                    break;
                }
            }
        }
    }

    return 0;
}

/**
 *\brief                    定时器线程
 *\param[in]    set         定时器管理者
 *\return                   空
 */
void* timer_thread(p_xt_timer_set set)
{
    D("begin\n");

    unsigned int now;
    unsigned int sec = 0;

    while (set->run)
    {
        msleep(100);

        now = (unsigned int)time(NULL);

        if (now == sec) // 线程是100毫秒执行1次,确保每1秒都会执行检测
        {
            continue;
        }

        sec = now;

        //D("%u\n", now);

        list_proc(&(set->timer_list), timer_check, (void*)now);
    }

    D("exit\n");
    return NULL;
}

/**
 *\brief                    定时器初始化
 *\param[in]    set         定时器管理者
 *\attention    set         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int timer_init(p_xt_timer_set set)
{
    if (NULL == set)
    {
        return -1;
    }

    int ret = list_init(&(set->timer_list));

    if (0 != ret)
    {
        return -2;
    }

    set->run = true;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    // 退出时自行释放所占用的资源

    ret = pthread_create(&tid, &attr, timer_thread, set);

    if (ret != 0)
    {
        E("create thread fail, E:%d\n", ret);
        return -3;
    }

    pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收

    D("ok\n");
    return 0;
}

/**
 *\brief                    定时器反初始化
 *\param[in]    set         定时器管理者
 *\return       0           成功
 */
int timer_uninit(p_xt_timer_set set)
{
    if (NULL == set)
    {
        return -1;
    }

    set->run = false;

    return list_uninit(&set->timer_list);
}

/**
 *\brief                    添加周期定时器
 *\param[in]    set         定时器管理者
 *\param[in]    name        定时器名称
 *\param[in]    cycle       定时器循环周期秒
 *\param[in]    thread_pool 线程池
 *\param[in]    task        任务回调函数
 *\param[in]    param       任务回调函数自定义参数,可以为NULLs
 *\return       0           成功
 */
int timer_add_cycle(p_xt_timer_set set, const char *name, unsigned int cycle,
                    p_xt_thread_pool thread_pool, XT_THREAD_POOL_TASK_CALLBACK task, void *param)
{
    if (NULL == set || NULL == name || 0 == cycle || NULL == thread_pool || NULL == task || !set->run || !thread_pool->run)
    {
        return -1;
    }

    p_xt_timer timer    = (p_xt_timer)malloc(sizeof(xt_timer));
    timer->type         = TIMER_TYPE_CYCLE;
    timer->cycle_second = cycle;
    timer->cycle_next   = (unsigned int)time(NULL);
    timer->thread_pool  = thread_pool;
    timer->task.proc    = task;
    timer->task.param   = param;
    strncpy_s(timer->name, sizeof(timer->name), name, sizeof(timer->name) - 1);

    return list_tail_push(&(set->timer_list), timer);
}

/**
 *\brief                    添加条件定时器
 *\param[in]    set         定时器管理者
 *\param[in]    name        定时器名称
 *\param[in]    type        定时器类型:TIMER_CRON_YDAY,TIMER_CRON_WDAY,...
 *\param[in]    yday        天/年 0-365
 *\param[in]    wday        星期  0-6
 *\param[in]    mon         月份  0-11
 *\param[in]    mday        天/月 1-31
 *\param[in]    hour        小时  0-23
 *\param[in]    min         分钟  0-59
 *\param[in]    sec         秒    0-59
 *\param[in]    thread_pool 线程池
 *\param[in]    task        任务回调函数
 *\param[in]    param       任务回调函数自定义参数,可以为NULL
 *\return       0           成功
 */
int timer_add_cron(p_xt_timer_set set,
                   const char *name, unsigned int type,
                   unsigned short yday, unsigned char wday,
                   unsigned char mon, unsigned char mday,
                   unsigned char hour, unsigned char min, unsigned char sec,
                   p_xt_thread_pool thread_pool, XT_THREAD_POOL_TASK_CALLBACK task, void *param)
{
    if (NULL == set || NULL == name ||
        type < TIMER_CRON_YDAY || type > TIMER_CRON_MINUTE ||
        yday > 365 || wday > 6 || mon > 11 || mday < 1 || mday > 31 || hour > 23 || min > 59 || sec > 59 ||
        NULL == thread_pool || NULL == task || !set->run || !thread_pool->run)
    {
        return -1;
    }

    p_xt_timer   timer  = (p_xt_timer)malloc(sizeof(xt_timer));
    timer->type         = type;
    timer->cron_yday    = yday;
    timer->cron_wday    = wday;
    timer->cron_mon     = mon;
    timer->cron_mday    = mday;
    timer->cron_hour    = hour;
    timer->cron_min     = min;
    timer->cron_sec     = sec;
    timer->thread_pool  = thread_pool;
    timer->task.proc    = task;
    timer->task.param   = param;
    strncpy_s(timer->name, sizeof(timer->name), name, sizeof(timer->name) - 1);

    return list_tail_push(&(set->timer_list), timer);
}
