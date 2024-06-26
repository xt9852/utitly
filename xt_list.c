/**
 *\file     xt_list.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2015.8.29
 *\brief    链表数据结构实现
 */
#include <stdlib.h>
#include "xt_list.h"

#ifdef XT_LOG
    #include "xt_log.h"
#else
    #include <stdio.h>
    #include <stdlib.h>
#ifdef _WINDOWS
    #define D(...)      printf(__VA_ARGS__)
    #define I(...)      printf(__VA_ARGS__)
    #define W(...)      printf(__VA_ARGS__)
    #define E(...)      printf(__VA_ARGS__)
#else
    #define D(args...)  printf(args)
    #define I(args...)  printf(args)
    #define W(args...)  printf(args)
    #define E(args...)  printf(args)
#endif
#endif

#define SV          sizeof(void*)   ///< void指针大小

#define NODE_SIZE   1024            ///< 节点初始大小

/**
 *\brief                链表初始化
 *\param[in]    list    链表
 *\return       0       成功
 */
int list_init(p_xt_list list)
{
    if (NULL == list)
    {
        return -1;
    }

    pthread_mutex_init(&(list->mutex), NULL);
    pthread_mutex_lock(&(list->mutex));

    list->count = 0;
    list->head = -1;    // 无数据
    list->tail = -1;    // 无数据
    list->size = NODE_SIZE;
    list->data = malloc(SV * list->size);

    pthread_mutex_unlock(&(list->mutex));
    return 0;
}

/**
 *\brief                链表反初始化
 *\param[in]    list    链表
 *\return       0       成功
 */
int list_uninit(p_xt_list list)
{
    if (NULL == list)
    {
        return -1;
    }

    list->count = 0;
    list->head = -1;    // 无数据
    list->tail = -1;    // 无数据
    list->size = 0;
    free(list->data);

    pthread_mutex_destroy(&(list->mutex));
    return 0;
}

/**
 *\brief                在链表尾部添加数据
 *\param[in]    list    链表
 *\param[in]    data    数据
 *\return       0       成功
 */
int list_tail_push(p_xt_list list, void *data)
{
    if (NULL == list)
    {
        return -1;
    }

    pthread_mutex_lock(&(list->mutex));

    if (0 == list->count)
    {
        list->head = 0;
        list->tail = 0;
        list->data[0] = data;
    }
    else if (list->count < list->size)
    {
        list->tail = (list->tail + 1) % list->size;
        list->data[list->tail] = data;
    }
    else
    {
        int    old_size = list->size;
        void **old_data = list->data;

        list->size = old_size + old_size / 2;   // 数据扩大一半
        list->data = malloc(SV * list->size);

        if (list->head <= list->tail)           // 头节点在尾节点前面
        {
            D("add array old size:%d head:%d <= tail:%d\n", old_size, list->head, list->tail);
            memcpy(list->data, old_data, SV * old_size);
            D("add array new size:%d head:%d tail:%d\n", list->size, list->head, list->tail);
        }
        else
        {
            D("add array old size:%d head:%d > tail:%d\n", old_size, list->head, list->tail);

            int cnt = old_size + list->head;
            int head = list->size - cnt;
            memcpy(list->data, old_data, SV * (list->tail + 1));
            memcpy(&list->data[list->size - cnt], &old_data[list->head], SV * cnt);
            list->head = head;

            D("add array new size:%d head:%d tail:%d\n", list->size, list->head, list->tail);
        }

        free(old_data);

        list->tail++;
        list->data[list->tail] = data;
    }

    list->count++;

    D("size:%d count:%d head:%d tail:%d\n", list->size, list->count, list->head, list->tail);

    pthread_mutex_unlock(&(list->mutex));

    return 0;
}

/**
 *\brief                从链表头部得到数据
 *\param[in]    list    链表
 *\param[out]   data    数据
 *\return       0       成功
 */
int list_head_pop(p_xt_list list, void **data)
{
    if (NULL == list || NULL == data)
    {
        return -1;
    }

    if (1 == list->count)
    {
        pthread_mutex_lock(&(list->mutex));

        *data = list->data[list->head];

        list->head = -1;
        list->tail = -1;
        list->count = 0;

        pthread_mutex_unlock(&(list->mutex));

        return 0;
    }

    if (list->count > 1)
    {
        pthread_mutex_lock(&(list->mutex));

        *data = list->data[list->head];
        list->head = (list->head + 1) % list->size;
        list->count--;

        pthread_mutex_unlock(&(list->mutex));

        return 0;
    }

    return -2;
}

/**
 *\brief                调用指定的回调函数遍历链表
 *\param[in]    list    链表
 *\param[in]    proc    回调函数
 *\param[in]    param   自定义数据
 *\return       0       成功\n
                1       无节点\n
                其它    失败
 */
int list_proc(p_xt_list list, LIST_PROC proc, void *param)
{
    if (NULL == list || NULL == proc)
    {
        return -1;
    }

    if (list->count <= 0)
    {
        return 1;
    }

    pthread_mutex_lock(&(list->mutex));

    for (int i = 0; i < list->count; i++)
    {
        proc(list->data[(list->head + i) % list->size], param);
    }

    pthread_mutex_unlock(&(list->mutex));

    return 0;
}
