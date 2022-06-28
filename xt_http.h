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
    HTTP_TYPE_HTML = 0, ///< 页面
    HTTP_TYPE_XML,      ///< XML
    HTTP_TYPE_ICO,      ///< 图标
    HTTP_TYPE_GIF,      ///< gif图片
    HTTP_TYPE_PNG,      ///< png图片
    HTTP_TYPE_JPG,      ///< jpg图片
    HTTP_TYPE_JPEG      ///< jpeg图片
};

/// URL参数
typedef struct _xt_http_arg
{
    int             count;              ///< 参数数量

    const char     *name[ARG_SIZE];     ///< 参数名称

    const char     *data[ARG_SIZE];     ///< 参数值

} xt_http_arg, *p_xt_http_arg;          ///< URL参数指针

/// 应答内容
typedef struct _xt_http_content
{
    int             type;               ///< 内容类型

    int             len;                ///< 内容数据长度

    char           *data;               ///< 内容数据

} xt_http_content, *p_xt_http_content;  ///< URL参数指针

/**
 *\brief        http回调函数
 *\param[in]    uri             URI地址
 *\param[in]    arg             URI的参数,参数使用的是conten.data指向的内存
 *\param[out]   content         返回内容
 *\return       200             成功
 */
typedef int (*XT_HTTP_CALLBACK)(const char *uri, const p_xt_http_arg arg, p_xt_http_content content);

/// HTTP服务数据
typedef struct _xt_http
{
    bool                run;            ///< 服务线程是否运行

    unsigned short      port;           ///< HTTP端口

    XT_HTTP_CALLBACK    proc;           ///< HTTP回调函数

    int                 listen_sock;    ///< 临听socket

} xt_http, *p_xt_http;                  ///< HTTP服务数据指针

/**
 *\brief        初始化http
 *\param[in]    port            监听端口
 *\param[in]    proc            处理请求回调
 *\param[in]    http            服务数据,需要run, port, proc
 *\attention    http            需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0               成功
 */
int http_init(unsigned short port, XT_HTTP_CALLBACK callback, p_xt_http http);

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
