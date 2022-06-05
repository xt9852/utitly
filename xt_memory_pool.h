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

#include "xt_list_stack.h"

/**
 *\brief        创建内存池
 *\param[in]    const char *name 池名称
 *\param[in]    int size 内存块大小
 *\return       0-成功
 */
int memory_pool_init(const char *name, int size);

/**
 *\brief        删除内存池
 *\return       0-成功
 */
int memory_pool_uninit();

/**
 *\brief        从内存池得到内存
 *\param[in]    unsigned int size 请求大小
 *\param[in]    void **mem 内存块
 *\return       int <0失败, >0内存块大小
 */
int memory_pool_get(unsigned int size, void **mem);

/**
 *\brief        回收内存到内存池
 *\param[in]    void * memory 内存块
 *\return       int 0-成功
 */
int memory_pool_put(void *mem);

/**
 *\brief        检测释放空闲内存
 *\param[in]    bool clean 是否释放空闲内存
 *\return       0-成功
 */
int memory_pool_check(bool clean);

/**
 *\brief        内存使用信息
 *\return       char* 信息
 */
char* memory_pool_info();

#endif
