/**
 *\file     xt_http.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.02.08
 *\brief    HTTP模块定义
 */
#ifndef _XT_HTTP_H_
#define _XT_HTTP_H_

#ifndef BOOL
#define bool unsigned char
#endif

#define ARG_SIZE        128                 ///< 参数数量

enum                                        ///  页面类型
{
    HTTP_TYPE_HTML = 0,                     ///< 页面
    HTTP_TYPE_XML,                          ///< XML
    HTTP_TYPE_ICO,                          ///< 图标
    HTTP_TYPE_GIF,                          ///< gif图片
    HTTP_TYPE_PNG,                          ///< png图片
    HTTP_TYPE_JPG,                          ///< jpg图片
    HTTP_TYPE_JPEG                          ///< jpeg图片
};

typedef struct _xt_http_arg                 ///  URL参数
{
    const char         *key;                ///< 参数名称

    const char         *value;              ///< 参数值

    unsigned int        value_len;          ///< 参数值长度

} xt_http_arg, *p_xt_http_arg;

typedef struct _xt_http_data                ///  HTTP数据
{
    const char         *uri;                ///< URI地址

    xt_http_arg         arg[ARG_SIZE];      ///< 参数

    unsigned int        arg_count;          ///< 参数数量

    unsigned int        type;               ///< 内容类型

    unsigned int        len;                ///< 输入数据缓冲区大小,输出内容数据长度

    char               *content;            ///< 内容数据

} xt_http_data, *p_xt_http_data;

/**
 *\brief                    http回调函数
 *\param[in]    uri         URI地址
 *\param[out]   data        HTTP的数据
 *\return       200         成功
 */
typedef int (*XT_HTTP_CALLBACK)(const p_xt_http_data data);


typedef struct _xt_http                     ///  HTTP服务数据
{
    bool                run;                ///< 服务线程是否运行

    char                ip[64];             ///< IP地址

    unsigned short      port;               ///< 端口

    bool                ipv4;               ///< IPV4

    XT_HTTP_CALLBACK    proc;               ///< HTTP回调函数

    int                 listen_sock;        ///< 临听socket

} xt_http, *p_xt_http;                      ///< HTTP服务数据指针

/**
 *\brief                    初始化http
 *\param[in]    ip          地址
 *\param[in]    port        监听端口
 *\param[in]    proc        处理请求回调
 *\param[in]    http        服务数据
 *\attention    http        需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int http_init(const char *ip, unsigned short port, XT_HTTP_CALLBACK proc, p_xt_http http);

#endif
