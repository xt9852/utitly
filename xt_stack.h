/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_stack.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 双向链表栈数据结构接口定义
*************************************************/

#ifndef _XT_STACK_H_
#define _XT_STACK_H_

typedef struct _stack_node
{
    int len;

    void *data;

    struct _node *next;

} stack_node, *p_stack_node;

typedef struct _stack
{
    unsigned int count;
    p_stack_node head;

} stack, *p_stack;

/**
 *\brief            栈初始化
 *\param[int|out]   p_stack stack 栈
 *\return           0-成功
 */
int stack_init(p_stack stack);

/**
 *\brief     节点入栈
 *\param[in] p_stack        stack   栈
 *\param[in] p_stack_node   node    栈节点
 *\return    0-成功
 */
int stack_push(p_stack stack, p_stack_node node);

/**
 *\brief     节点出栈
 *\param[in] p_stack stack 栈
 *\return    栈节点
 */
p_node stack_pop(p_stack stack);

#endif
