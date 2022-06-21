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

/// 页面类型
enum
{
    HTTP_TYPE_HTML = 0, ///< 页面
    HTTP_TYPE_ICON      ///< 图标
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
typedef int (*XT_HTTP_PROCESS)(const char *uri, const char *arg_name[], const char *arg_data[], int arg_count,
                               int *content_type, char *content, int *content_len);

/// HTTP服务数据
typedef struct _xt_http
{
    bool            run;            ///< 服务线程是否运行

    unsigned short  port;           ///< HTTP端口

    XT_HTTP_PROCESS proc;           ///< HTTP回调函数

    int             listen_sock;    ///< 临听socket

} xt_http, *p_xt_http;              ///< HTTP服务数据指针

/**
 *\brief        初始化http
 *\param[in]    http            服务数据,需要run, port, proc
 *\attention    http            需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0               成功
 */
int http_init(p_xt_http http);

#endif
