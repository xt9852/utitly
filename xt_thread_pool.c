/**
 *\file     xt_thread_pool.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2013.8.16
 *\brief    线程池模块实现
 */
#include "xt_thread_pool.h"

/**
 *\brief                线程池线程
 *\return               空
 */
void* thread_pool_thread(p_xt_thread_pool pool)
{
    D("begin");

    p_xt_thread_pool_task task;

    while(pool->run)
    {
        task = NULL;

        if (0 != list_head_pop(&(pool->task_queue), &task))
        {
            msleep(5);
            continue;
        }

        if (NULL != task)
        {
            pool->thread_count++;
            task->proc(task->param);
            pool->thread_count--;
        }
        else
        {
            E("get task null");
        }
    }

    D("exit");
    return NULL;
}

/**
 *\brief                线程池初始化
 *\param[in]    pool    线程池
 *\attention    pool    需要转递到线线程中,不要释放此内存,否则会野指针
 *\param[in]    count   线程数量
 *\return       0       成功
 */
int thread_pool_init(p_xt_thread_pool pool, unsigned int count)
{
    if (NULL == pool)
    {
        return -1;
    }

    int ret = list_init(&(pool->task_queue));

    if (0 != ret)
    {
        return -2;
    }

    pool->run           = true;
    pool->thread_count  = count;
    pool->process_count = 0;

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);            // 进程内竞争CPU
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    // 退出时自行释放所占用的资源

    for (unsigned int i = 0; i < count; i++)
    {
        ret = pthread_create(&tid, &attr, thread_pool_thread, pool);

        if (ret != 0)
        {
            E("create thread fail, E:%d\n", ret);
            return -3;
        }

        pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收
    }

    D("ok");
    return 0;
}

/**
 *\brief                删除线程池任务
 *\param[in]    task    任务
 *\param[in]    param   自定义参数
 *\return       0
 */
int thread_pool_del_task(p_xt_thread_pool_task task, void *param)
{
    free(task);
    return 0;
}

/**
 *\brief                线程池反初始化
 *\param[in]    pool    线程池
 *\return       0       成功
 */
int thread_pool_uninit(p_xt_thread_pool pool)
{
    if (NULL == pool)
    {
        return -1;
    }

    pool->run = false;
    list_proc(&(pool->task_queue), thread_pool_del_task, NULL);
    return 0;
}

/**
 *\brief                添加任务
 *\param[in]    pool    线程池
 *\param[in]    proc    任务回调接口
 *\param[in]    param   任务回调接口参数
 *\return       0       成功
 */
int thread_pool_put(p_xt_thread_pool pool, XT_THREAD_POOL_TASK_CALLBACK proc, void *param)
{
    if (NULL == pool || NULL == proc)
    {
        return -1;
    }

    p_xt_thread_pool_task task = (p_xt_thread_pool_task)malloc(sizeof(xt_thread_pool_task));
    task->proc  = proc;
    task->param = param;
    return list_tail_push(&(pool->task_queue), task);
}
