/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_list.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 双向链表列表数据结构接口定义,线程安全
*************************************************/

#ifndef _XT_LIST_WITH_LOCK_H_
#define _XT_LIST_WITH_LOCK_H_
#include "xt_stack.h"


typedef struct _list_with_lock_node
{
    char name[64];
    p_list list;
    p_stack free_node;
    pthread_mutex_t lock;
    unsigned int node_count;
    unsigned int node_stat;         // 空闲节点统计,数据不准确,用于释放多余内存
    unsigned int node_stat_times;   // 统计次数

} list_with_lock_node, *p_list_with_lock_node;

/**
 *\param[in]    p_list_with_lock_node list 链表的头结点
 *\param[in]    const char *name 链表名
 *\return       0-成功
 */
int list_with_lock_init(p_list_with_lock_node list, const char *name);

/**
 *\param[in]    p_list_with_lock_node list 链表的头结点
 *\return       0-成功
 */
int list_with_lock_uninit(p_list_with_lock_node list);

/**
 *\param[in]    p_list_with_lock_node list 链表的头结点
 *\param[in]    void *data 数据
 *\return       0-成功
 */
int list_with_lock_push(p_list_with_lock_node list, void *data);

/**
 *\param[in]    p_list_with_lock_node list 链表的头结点
 *\return       void *data 数据
 */
void* list_with_lock_pop(p_list_with_lock_node list);

/**
 *\param[in]    p_list_with_lock_node list 链表的头结点
 *\param[in]    bool clean 是否清除节点
 *\return       0-成功
 */
int list_check(p_list_with_lock_node list, bool clean);

#endif
