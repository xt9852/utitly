/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_list.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 链表数据结构接口定义
*************************************************/

#ifndef _XT_LIST_STACK_H_
#define _XT_LIST_STACK_H_

#include <pthread.h>

typedef struct _list
{
    void          **data;   // 指向数据数组

    int             size;   // 数据数组大小

    int             count;  // 当前节点数量

    int             head;   // 头节点数组下标

    int             tail;   // 尾节点数组下标

    pthread_mutex_t mutex;  // 线程锁

} list, *p_list;

/**
 *\brief        链表初始化
 *\param[in]    p_list  list    链表
 *\return       0-成功
 */
int list_init(p_list list);

/**
 *\brief        链表反初始化
 *\param[in]    p_list  list    链表
 *\return       0-成功
 */
int list_uninit(p_list list);

/**
 *\brief        在链表尾部添加数据
 *\param[in]    p_list  list    链表
 *\param[in]    void    *data   数据
 *\return       0-成功
 */
int list_tail_push(p_list list, void *data);

/**
 *\brief        从链表头部得到数据
 *\param[in]    p_list  list    链表
 *\param[out]   void  **list    数据
 *\return       0-成功
 */
int list_head_pop(p_list list, void **data);

/**
 *\brief        回调函数
 *\param[in]    void   *data    链表数据
 *\param[in]    void   *param   自定义数据
 *\return       0-成功将继续调用下一个节点,其它将退出循环
 */
typedef int (*LIST_PROC)(void *data, void *param);

/**
 *\brief        调用指定的回调函数遍历链表
 *\param[in]    p_list          list    链表
 *\param[in]    LIST_PROC       proc    回调函数
 *\param[in]    void           *param   自定义数据
 *\return       0-成功
 */
int list_proc(p_list list, LIST_PROC proc, void *param);

#endif
