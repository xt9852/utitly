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

enum
{
    HTTP_TYPE_HTML = 0,
    HTTP_TYPE_ICON
};

/**
 *\brief        http回调函数
 *\param[in]    uri             URI地址
 *\param[in]    arg_name        URI的参数名称
 *\param[in]    arg_data        URI的参数数据
 *\param[in]    arg_count       URI的参数数量
 *\param[out]   content_type    返回内容类型
 *\param[out]   content         返回内容
 *\param[out]   content_len     返回内容长度
 *\return       200             成功
 */
typedef int (*XT_HTTP_PROCESS)(const char *uri, const char **arg_name, const char **arg_data, int arg_count,
                               int *content_type, char *content, int *content_len);

/// http服务
typedef struct _xt_http
{
    bool run;

    int listen_sock;

    unsigned short  port;

    XT_HTTP_PROCESS proc;

} xt_http, *p_xt_http;

/**
 *\brief        初始化http
 *\param[in]    http        http服务服务数据,需要port, proc
 *\return       0           成功
 */
int http_init(p_xt_http http);

#endif
