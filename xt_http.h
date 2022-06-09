/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_http.h
 *\author       xt
 *\version      1.0.0
 *\date         2022-02-08
 *\brief        HTTP模块接口定义,UTF-8(No BOM)
 */
#ifndef _XT_HTTP_H_
#define _XT_HTTP_H_

/**
 * \brief       http回调函数
 * \param[in]   uri         URI地址
 * \param[in]   arg         参数
 * \param[in]   arg_count   参数数量
 * \param[out]  content     返回内容
 * \param[out]  content_len 返回内容长度
 * \return      0           成功
 */
typedef int (*HTTP_PROCESS)(const char *uri, const char *arg, int arg_count, char *content, int *content_len);


/**
 * \brief       初始化http
 * \param[in]   port    端口号
 * \return      0       成功
 */
int http_init(int port, HTTP_PROCESS proc);

#endif
