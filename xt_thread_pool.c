/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_thread_pool.c
 *\author       xt
 *\version      1.0.0
 *\date         2013.8.16
 *\brief        线程池模块实现,UTF-8(No BOM)
 */
#include "xt_thread_pool.h"

/**
 *\brief     线程池线程
 *\return    空
 */
void* thread_pool_thread(void *param)
{
    DBG("begin");

    p_task task;
    p_thread_pool pool = (p_thread_pool)param;

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
            ERR("get task null");
        }
    }

    DBG("exit");
    return NULL;
}

/**
 *\brief        线程池初始化
 *\param[in]    pool    线程池
 *\param[in]    count   线程数量
 *\return       0       成功
 */
int thread_pool_init(p_thread_pool pool, unsigned int count)
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
            ERR("create thread fail, err:%d\n", ret);
            return -3;
        }
    }

    DBG("ok");
    return 0;
}

/**
 *\brief        线程池反初始化
 *\param[in]    pool    线程池
 *\return       0       成功
 */
int thread_pool_uninit(p_thread_pool pool)
{
    if (NULL == pool)
    {
        return -1;
    }

    pool->run = false;

    return 0;
}

/**
 *\brief        添加任务
 *\param[in]    pool    线程池
 *\param[in]    proc    任务回调接口
 *\param[in]    param   任务回调接口参数
 *\return       0       成功
 */
int thread_pool_put(p_thread_pool pool, THREAD_POOL_CALLBACK proc, void *param)
{
    if (NULL == pool || NULL == proc)
    {
        return -1;
    }

    p_task task = (p_task)malloc(sizeof(task));
    task->proc  = proc;
    task->param = param;
    return list_tail_push(&(pool->task_queue), task);
}

/*
static lim_thread_job* get_job(p_lim_thread_pool pool)
{
    p_lim_thread_job job = NULL;

#ifdef USE_MUTEX
    {
        p_list_node node = NULL;

        pthread_mutex_lock(&pool->job_lock);
        node = list_pop(pool->job_queue);
        pthread_mutex_unlock(&pool->job_lock);

        if (NULL != node)
        {
            job = (p_lim_thread_job)node->pdata;
            node_release(node, NULL);
        }
    }
#else
    job = (p_lim_thread_job)lock_free_list_pop(pool->job_queue);
#endif

    return job;
}

static void* exec_job(p_lim_thread_job job)
{
    if (NULL == job) return NULL;

    job->action(job->param);

    return NULL;
}

static int res_set_leader_thread(const p_lim_thread_pool pool)
{
    int i = 0;
    int id = 0;
    int leader = pool->cur_leader_id;
    p_lim_thread_info thread_array = pool->thread_array;

    if (pool->cur_process_count < (pool->thread_count-1))
    {
        for (i = 1; i < pool->thread_count; i++)
        {
            id = (leader + i) % pool->thread_count;

            if (thread_array[id].type == TT_FOLLWER)
            {
                thread_array[id].type = TT_LEADER;
                pool->cur_leader_id = id;
                return 0;
            }
        }
    }

    return E_TP_NO_THREAD;
}

static void* threadpool_func(void *param)
{
    int ret = 0;
    uint64 end = 0;
    uint64 begin = 0;
    uint64 time[2] = {0};
    p_list_node node = NULL;
    p_lim_thread_job job = NULL;
    p_lim_thread_pool pool = NULL;
    p_lim_thread_info thread_info = (p_lim_thread_info)param;

    if (NULL == param)
    {
        return NULL;
    }

    pool = (p_lim_thread_pool)thread_info->thread_pool;

    while (pool->working)
    {
#ifdef USE_LEADER_FOLLOWER
        //是否是Leader
        if (TT_LEADER != thread_info->type)
        {
            lim_sleep(1);
            continue;
        }
#endif

        //得到任务
        if (NULL == (job = get_job(pool)))
        {
            lim_sleep(1);
            continue;
        }

        thread_info->job_push_pop_time += (lim_get_cur_time_ms() - job->begin_time);
        thread_info->type = TT_PROCESSER;

#ifdef USE_LEADER_FOLLOWER
        //设置leader
        ret = res_set_leader_thread(pool);
#endif

        atomic_inc(pool->cur_process_count);
        atomic_inc(g_thread_pool_info.cur_processer_count);

        begin = lim_get_cur_time_ms();

        exec_job(job);

        end = lim_get_cur_time_ms();

        time[0] = end - begin;
        time[1] = end - job->begin_time;

        thread_info->process_job_count++;
        thread_info->process_job_time += time[0];
        thread_info->process_job_total_time += time[1];
        thread_info->type = (0 == ret) ? TT_FOLLWER : TT_LEADER;

        atomic_dec(pool->cur_process_count);
        atomic_dec(g_thread_pool_info.cur_processer_count);
        atomic_inc(g_thread_pool_info.process_job_count);
        atomic_add(g_thread_pool_info.process_job_time, time[0]);
        atomic_add(g_thread_pool_info.process_job_total_time, time[1]);

        free(job);
    }

    atomic_dec(pool->thread_count);

    return 0;
}

int lim_create_threadpool(p_lim_thread_pool *pool, const int thread_count)
{
    uint i = 0;
    uint count = 0;
    uint logic_cpu_num = sysi_logic_cpu_num();
    pthread_attr_t attr = {0};
    p_lim_thread_info thread_info = NULL;
    p_lim_thread_pool thread_pool = NULL;

    if (NULL == pool || thread_count <= 0)
    {
        lim_write_log(LOG_LEVEL_ERROR, "%s param is null", __FUNCTION__);
        return E_TP_PARAM_NULL;
    }

    thread_pool = (p_lim_thread_pool)lim_malloc(sizeof(lim_thread_pool));
    memset(thread_pool, 0, sizeof(lim_thread_pool));
    thread_pool->working = true;

#ifdef USE_MUTEX
    pthread_mutex_init(&(thread_pool->job_lock), NULL);
    thread_pool->job_queue = create_node(NULL);
    list_init(thread_pool->job_queue);
#else
    lock_free_list_create(&(thread_pool->job_queue));
#endif

    count = (thread_count < logic_cpu_num) ? logic_cpu_num : thread_count;
    thread_pool->thread_count = count;

    thread_pool->thread_array = (p_lim_thread_info)lim_malloc(sizeof(lim_thread_info)*count);
    memset(thread_pool->thread_array, 0, sizeof(lim_thread_info)*count);

    pthread_attr_t attr = {0};
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS); //进程内竞争CPU
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //退出时自行释放所占用的资源

    for (i = 0; i < count; ++i)
    {
        thread_info = &(thread_pool->thread_array[i]);
        thread_info->thread_pool = thread_pool;

        if (0 != pthread_create(&thread_info->tid, &attr, threadpool_func, thread_info))
        {
            break;
        }
    }

    thread_pool->thread_array[0].type = TT_LEADER;

    *pool = thread_pool;
    atomic_inc(g_thread_pool_info.thread_pool_count);
    atomic_add(g_thread_pool_info.thread_total_count, count);

    return 0;
}

int lim_destroy_threadpool(p_lim_thread_pool pool)
{
    if (NULL == pool)
    {
        lim_write_log(LOG_LEVEL_ERROR, "%s param is null", __FUNCTION__);
        return E_TP_PARAM_NULL;
    }
    else if (pool == g_public_thread_pool)
    {
        lim_write_log(LOG_LEVEL_DEBUG, "%s destroy public thread pool", __FUNCTION__);
        return 0;
    }

    pool->working = false;

    while (pool->thread_count > 0)
    {
        lim_sleep(10); //等待全部线程退出后清资源
    }

    atomic_dec(g_thread_pool_info.thread_pool_count);
    atomic_sub(g_thread_pool_info.thread_total_count, pool->thread_count);

#ifdef USE_MUTEX
    pthread_mutex_lock(&pool->job_lock);
    list_destroy(pool->job_queue, lim_release_node);
    pthread_mutex_unlock(&pool->job_lock);
    pthread_mutex_destroy(&pool->job_lock);
#endif

    free(pool->thread_array);

    return 0;
}

int lim_push_job(p_lim_thread_pool pool, TP_CALLBACK action, void *param)
{
    p_lim_thread_job job = NULL;

    if (NULL == pool || NULL == action)
    {
        lim_write_log(LOG_LEVEL_ERROR, "%s param is null", __FUNCTION__);
        return E_TP_PARAM_NULL;
    }

    job = (p_lim_thread_job)lim_malloc(sizeof(lim_thread_job));
    job->param = param;
    job->action = action;
    job->begin_time = lim_get_cur_time_ms();

#ifdef USE_MUTEX
    {
        p_list_node node = (p_list_node)lim_malloc(sizeof(list_node));
        node_init(node, job);

        pthread_mutex_lock(&pool->job_lock);
        list_push(list_tail(pool->job_queue), node);
        pthread_mutex_unlock(&pool->job_lock);
    }
#else
    lock_free_list_push(pool->job_queue, job);
#endif

    return 0;
}

int lim_thread_pool_info(p_thread_pool_info info)
{
    if (NULL == info)
    {
        lim_write_log(LOG_LEVEL_ERROR, "%s param is null", __FUNCTION__);
        return E_TP_PARAM_NULL;
    }

    *info = g_thread_pool_info;

    return 0;
}

int lim_get_public_thread_pool(p_lim_thread_pool *thread_pool, const int thread_count)
{
    int ret = 0;

    if (NULL == thread_pool || thread_count <= 0)
    {
        lim_write_log(LOG_LEVEL_ERROR, "%s param is null", __FUNCTION__);
        return E_TP_PARAM_NULL;
    }

    if (NULL == g_public_thread_pool)
    {
        ret = lim_create_threadpool(&g_public_thread_pool, thread_count);
    }

    *thread_pool = g_public_thread_pool;

    return ret;
}
*/