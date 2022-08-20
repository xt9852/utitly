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

/// 参数数量
#define ARG_SIZE        128

/// 页面类型
enum
{
    HTTP_TYPE_HTML = 0,                     ///< 页面
    HTTP_TYPE_XML,                          ///< XML
    HTTP_TYPE_ICO,                          ///< 图标
    HTTP_TYPE_GIF,                          ///< gif图片
    HTTP_TYPE_PNG,                          ///< png图片
    HTTP_TYPE_JPG,                          ///< jpg图片
    HTTP_TYPE_JPEG                          ///< jpeg图片
};

/// URL参数
typedef struct _xt_http_arg
{
    const char         *key;                ///< 参数名称

    const char         *value;              ///< 参数值
    unsigned int        value_len;          ///< 参数值长度

} xt_http_arg, *p_xt_http_arg;              ///< URL参数指针

/// HTTP数据
typedef struct _xt_http_data
{
    const char         *uri;                ///< URI地址

    xt_http_arg         arg[ARG_SIZE];      ///< 参数

    unsigned int        arg_count;          ///< 参数数量

    unsigned int        type;               ///< 内容类型

    unsigned int        len;                ///< 输入数据缓冲区大小,输出内容数据长度

    char               *content;            ///< 内容数据

} xt_http_data, *p_xt_http_data;            ///< HTTP数据指针

/**
 *\brief        http回调函数
 *\param[in]    uri             URI地址
 *\param[out]   data            HTTP的数据
 *\return       200             成功
 */
typedef int (*XT_HTTP_CALLBACK)(const p_xt_http_data data);

/// HTTP服务数据
typedef struct _xt_http
{
    bool                run;                ///< 服务线程是否运行

    char                ip[64];             ///< IP地址

    unsigned short      port;               ///< 端口

    bool                ipv4;               ///< IPV4

    XT_HTTP_CALLBACK    proc;               ///< HTTP回调函数

    int                 listen_sock;        ///< 临听socket

} xt_http, *p_xt_http;                      ///< HTTP服务数据指针

/**
 *\brief        初始化http
 *\param[in]    ip              地址
 *\param[in]    port            监听端口
 *\param[in]    proc            处理请求回调
 *\param[in]    http            服务数据
 *\attention    http            需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0               成功
 */
int http_init(const char *ip, unsigned short port, XT_HTTP_CALLBACK proc, p_xt_http http);

/**
 *\brief        URI编码
 *\param[in]    in          原始的数据
 *\param[in]    in_len      原始的数据长度
 *\param[out]   out         编码后数据
 *\param[out]   out_len     输入数据缓冲区大小,输出编码后数据长度
 *\return       0           成功
 */
int uri_encode(const char *in, int in_len, char *out, int *out_len);

/**
 *\brief        URI解码
 *\param[in]    in          URI数据
 *\param[in]    in_len      URI数据长度
 *\param[out]   out         原始数据
 *\param[out]   out_len     输入数据缓冲区大小,输出解码后数据长度
 *\return       0           成功
 */
int uri_decode(char *in, int in_len, char *out, int *out_len);

#endif
