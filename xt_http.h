/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_http.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: HTTP模块接口定义
*************************************************/

#ifndef _XT_HTTP_H_
#define _XT_HTTP_H_


typedef int (*HTTP_PROCESS)(const char *uri, const char *arg, int arg_count, char *content, int *content_len);


/**
 * \brief      初始化http
 * \param[in]  int port      端口号
 * \return     0-成功
 */
int http_init(int port, HTTP_PROCESS proc);

#endif
