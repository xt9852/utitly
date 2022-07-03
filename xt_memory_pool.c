/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_memory_pool.c
 *\author       xt
 *\version      1.0.0
 *\date         2015.8.29
 *\brief        内存池模块实现,UTF-8(No BOM)
 */
#include "xt_memory_pool.h"
#include <stdlib.h>

/**
 *\brief        初始化内存池
 *\param[in]    pool    池
 *\param[in]    size    内存块大小
 *\param[in]    count   初始内存块数量,要大于1,后续每次增加其一半
 *\return       0       成功
 */
int memory_pool_init(p_xt_memory_pool pool, int size, int count)
{
    if (NULL == pool || size <= 0 || count <= 1)
    {
        return -1;
    }

    pool->count = count;

    pool->mem_size = size;

    list_init(&(pool->free));

    for (int i = 0; i < count; i++)
    {
        list_tail_push(&(pool->free), malloc(size));
    }

    return 0;
}

/**
 *\brief        链表删除内存
 *\param[in]    mem     内存
 *\param[in]    param   参数
 *\return       0       成功
 */
int LIST_DEL_MEM(void *mem, void *param)
{
    free(mem);
    return 0;
}

/**
 *\brief        反初始化内存池
 *\param[in]    pool    池
 *\return       0       成功
 */
int memory_pool_uninit(p_xt_memory_pool pool)
{
    if (NULL == pool)
    {
        return -1;
    }

    list_proc(&(pool->free), LIST_DEL_MEM, NULL);

    list_uninit(&(pool->free));

    pool->count = 0;

    pool->mem_size = 0;

    return 0;
}

/**
 *\brief        从内存池得到内存
 *\param[in]    pool    池
 *\param[in]    mem     内存块
 *\return       0       成功
 */
int memory_pool_get(p_xt_memory_pool pool, void **mem)
{
    if (NULL == pool || NULL == mem)
    {
        return -1;
    }

    int ret = list_head_pop(&(pool->free), mem);

    if (-2 == ret)   // 没有取到数据
    {
        *mem = malloc(pool->mem_size);

        int count = pool->count / 2;    // 添加已使用的数量的一半

        for (int i = 0; i < count; i++)
        {
            list_tail_push(&(pool->free), malloc(pool->mem_size));
        }

        pool->count += count;
    }

    return 0;
}

/**
 *\brief        回收内存到内存池
 *\param[in]    pool    池
 *\param[in]    mem     内存块
 *\return       0       成功
 */
int memory_pool_put(p_xt_memory_pool pool, void *mem)
{
    if (NULL == pool || NULL == mem)
    {
        return -1;
    }

    list_tail_push(&(pool->free), mem);

    return 0;
}
