/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_http.h
 *\author       xt
 *\version      1.0.0
 *\date         2022.02.08
 *\brief        HTTP模块定义,UTF-8(No BOM)
 */
#ifndef _XT_HTTP_H_
#define _XT_HTTP_H_
#include "xt_log.h"

/**
 * \brief       http回调函数
 * \param[in]   uri         URI地址
 * \param[in]   arg         参数
 * \param[in]   arg_count   参数数量
 * \param[out]  content     返回内容
 * \param[out]  content_len 返回内容长度
 * \return      0           成功
 */
typedef int (*XT_HTTP_PROCESS)(const char *uri, const char *arg, int arg_count, char *content, int *content_len);

/// http服务
typedef struct _xt_http
{
    bool run;

    int listen_sock;

    int client_sock;

    unsigned short  port;

    XT_HTTP_PROCESS proc;

} xt_http, *p_xt_http;

/**
 * \brief       初始化http
 * \param[in]   http    http服务服务数据,需要port, proc
 * \return      0       成功
 */
int http_init(p_xt_http http);

#endif
