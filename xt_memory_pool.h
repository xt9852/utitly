/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_memory_pool.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 内存池接口定义
*************************************************/

#ifndef _XT_MEMORY_POOL_H_
#define _XT_MEMORY_POOL_H_

#include "xt_list.h"

typedef struct _memory_pool
{
    int     mem_size;   // 内存块大小
    int     count;      // 内存数量
    p_list  free;       // 空闲的内存块链表

} memory_pool, *p_memory_pool;

/**
 *\brief        初始化内存池
 *\param[in]    p_memory_pool   pool    池
 *\param[in]    int             size    内存块大小
 *\param[in]    int             count   初始内存块数量
 *\return       0-成功
 */
int memory_pool_init(p_memory_pool pool, int size, int count);

/**
 *\brief        反初始化内存池
 *\param[in]    p_memory_pool   pool    池
 *\return       0-成功
 */
int memory_pool_uninit(p_memory_pool pool);

/**
 *\brief        从内存池得到内存
 *\param[in]    p_memory_pool   pool    池
 *\param[in]    void          **mem     内存块
 *\return       0-成功
 */
int memory_pool_get(p_memory_pool pool, void **mem);

/**
 *\brief        回收内存到内存池
 *\param[in]    p_memory_pool   pool    池
 *\param[in]    void           *mem     内存块
 *\return       0-成功
 */
int memory_pool_put(p_memory_pool pool, void *mem);

#endif
