/**
 *\file     xt_thread_pool.h
 *\author   xt
 *\version  1.0.0
 *\date     2013.8.16
 *\brief    线程池模块定义
 */
#ifndef _XT_THEAD_POOL_H
#define _XT_THEAD_POOL_H
#include "xt_log.h"
#include "xt_list.h"

typedef void (*XT_THREAD_POOL_TASK_CALLBACK)(void*);    ///< 线程池回调接口


typedef struct _xt_thread_pool_task                     ///  程任务数据
{
    XT_THREAD_POOL_TASK_CALLBACK    proc;               ///< 任务回调

    void*                           param;              ///< 任务回调参数

} xt_thread_pool_task, *p_xt_thread_pool_task;

typedef struct _xt_thread_pool                          ///  程池数据
{
    bool            run;                                ///< 线程是否运行

    unsigned int    thread_count;                       ///< 线程数量

    unsigned int    process_count;                      ///< 当前处理任务线程数量

    xt_list         task_queue;                         ///< 任务队列

} xt_thread_pool, *p_xt_thread_pool;

/**
 *\brief                线程池初始化
 *\param[in]    pool    线程池
 *\attention    pool    需要转递到线线程中,不要释放此内存,否则会野指针
 *\param[in]    count   线程数量
 *\return       0       成功
 */
int thread_pool_init(p_xt_thread_pool pool, unsigned int count);

/**
 *\brief                 线程池反初始化
 *\param[in]    pool    线程池
 *\return       0       成功
 */
int thread_pool_uninit(p_xt_thread_pool pool);

/**
 *\brief                添加任务
 *\param[in]    pool    线程池
 *\param[in]    proc    任务回调接口
 *\param[in]    param   任务回调接口参数
 *\return       0       成功
 */
int thread_pool_put(p_xt_thread_pool pool, XT_THREAD_POOL_TASK_CALLBACK proc, void *param);

#endif
