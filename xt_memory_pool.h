/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_memory_pool.h
 *\author       xt
 *\version      1.0.0
 *\date         2015.8.29
 *\brief        内存池模块定义,UTF-8(No BOM)
 */
#ifndef _XT_MEMORY_POOL_H_
#define _XT_MEMORY_POOL_H_

#include "xt_list.h"

/// 内存池
typedef struct _xt_memory_pool
{
    int     mem_size;                   ///< 内存块大小
    int     count;                      ///< 总分配内存块数
    xt_list free;                       ///< 空闲的内存块链表

} xt_memory_pool, *p_xt_memory_pool;    ///< 内存池类型

/**
 *\brief        初始化内存池
 *\param[in]    pool    池
 *\param[in]    size    内存块大小
 *\param[in]    count   初始内存块数量
 *\return       0       成功
 */
int memory_pool_init(p_xt_memory_pool pool, int size, int count);

/**
 *\brief        反初始化内存池
 *\param[in]    pool    池
 *\return       0       成功
 */
int memory_pool_uninit(p_xt_memory_pool pool);

/**
 *\brief        从内存池得到内存
 *\param[in]    pool    池
 *\param[in]    mem     内存块
 *\return       0       成功
 */
int memory_pool_get(p_xt_memory_pool pool, void **mem);

/**
 *\brief        回收内存到内存池
 *\param[in]    pool    池
 *\param[in]    mem     内存块
 *\return       0       成功
 */
int memory_pool_put(p_xt_memory_pool pool, void *mem);

#endif
