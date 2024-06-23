/**
 *\file     xt_timer.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2014.6.24
 *\brief    定时器模块器定义
 */
#ifndef _XT_TIMER_H_
#define _XT_TIMER_H_
#include "xt_thread_pool.h"

/// 定时器类型
enum
{
    TIMER_TYPE_CYCLE,                       ///< 按周期执行
    TIMER_TYPE_CRON,                        ///< 按条件执行
    TIMER_CRON_YDAY,                        ///< 每年的第yday天的hour时min分sec秒执行任务
    TIMER_CRON_WDAY,                        ///< 每周的第wday天的hour时min分sec秒执行任务
    TIMER_CRON_YEAR,                        ///< 每年的第mon月第mday天的hour时min分sec秒执行任务
    TIMER_CRON_MON,                         ///< 每月的第mday天的hour时min分sec秒执行任务
    TIMER_CRON_DAY,                         ///< 每天的hour时min分sec秒执行任务
    TIMER_CRON_HOUR,                        ///< 每时的min分sec秒执行任务
    TIMER_CRON_MINUTE                       ///< 每分的sec秒执行任务
};

typedef struct _xt_timer *p_xt_timer;


typedef struct _xt_timer                    ///  时器数据结构
{
    char                    name[64];       ///< 自定义定时器名称
    int                     type;           ///< 定时器类型:TIMER_TYPE_CYCLE,TIMER_CRON_YDAY,...

    unsigned int            cycle_second;   ///< 周期间隔时间秒
    unsigned int            cycle_next;     ///< 下一周期时间

    unsigned short          cron_yday;      ///< 天/年 0-365 0-一月一日
    unsigned char           cron_wday;      ///< 星期  0-6   0-星期日
    unsigned char           cron_mon;       ///< 月份  0-11  0-一月
    unsigned char           cron_mday;      ///< 天/月 1-31  1-一日
    unsigned char           cron_hour;      ///< 小时  0-23  0-二十四时
    unsigned char           cron_min;       ///< 分钟  0-59
    unsigned char           cron_sec;       ///< 秒    0-59

    p_xt_thread_pool        thread_pool;    ///< 处理任务的线程池
    xt_thread_pool_task     task;           ///< 任务

} xt_timer, *p_xt_timer;

typedef struct _xt_timer_set                ///  时器数据结构
{
    bool                    run;            ///< 定时器线程是否运行

    xt_list                 timer_list;     ///< 定时器列表

} xt_timer_set, *p_xt_timer_set;            ///< 定时器数据结构指针

/**
 *\brief                    定时器初始化
 *\param[in]    set         定时器管理者
 *\attention    set         需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int timer_init(p_xt_timer_set set);

/**
 *\brief                    定时器反初始化
 *\param[in]    set         定时器管理者
 *\return       0           成功
 */
int timer_uninit(p_xt_timer_set set);

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
                    p_xt_thread_pool thread_pool, XT_THREAD_POOL_TASK_CALLBACK task, void *param);

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
                   p_xt_thread_pool thread_pool, XT_THREAD_POOL_TASK_CALLBACK task, void *param);

#endif
