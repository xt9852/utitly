/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_list.h
 *\author       xt
 *\version      1.0.0
 *\date         2015.8.29
 *\brief        链表数据结构定义,UTF-8(No BOM)
 */
#ifndef _XT_LIST_H_
#define _XT_LIST_H_

#include <pthread.h>

/// 链表
typedef struct _xt_list
{
    void          **data;       ///< 指向数据数组

    int             size;       ///< 数据数组大小

    int             count;      ///< 当前节点数量

    int             head;       ///< 头节点数组下标

    int             tail;       ///< 尾节点数组下标

    pthread_mutex_t mutex;      ///< 线程锁

} xt_list, *p_xt_list;          ///< 链表类型

/**
 *\brief        链表初始化
 *\param[in]    list    链表
 *\return       0       成功
 */
int list_init(p_xt_list list);

/**
 *\brief        链表反初始化
 *\param[in]    list    链表
 *\return       0       成功
 */
int list_uninit(p_xt_list list);

/**
 *\brief        在链表尾部添加数据
 *\param[in]    list    链表
 *\param[in]    data    数据
 *\return       0       成功
 */
int list_tail_push(p_xt_list list, void *data);

/**
 *\brief        从链表头部得到数据
 *\param[in]    list    链表
 *\param[out]   list    数据
 *\return       0       成功
 */
int list_head_pop(p_xt_list list, void **data);

/**
 *\brief        回调函数
 *\param[in]    data    链表数据
 *\param[in]    param   自定义数据
 *\return       0       成功将继续调用下一个节点\n
                        其它将退出循环
 */
typedef int (*LIST_PROC)(void *data, void *param);

/**
 *\brief        调用指定的回调函数遍历链表
 *\param[in]    list    链表
 *\param[in]    proc    回调函数
 *\param[in]    param   自定义数据
 *\return       0       成功
 */
int list_proc(p_xt_list list, LIST_PROC proc, void *param);

#endif
