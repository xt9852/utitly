#pragma once // 只编译一次
#include "xt_http.h"


/**
 * \brief      初始化http
 * \param[in]  char          *buff      缓存
 * \param[in]  uint           size      缓存大小
 * \param[in]  p_config_http  http
 * \param[in]  p_config_wifi  wifi
 * \param[in]  p_config_light light
 * \return     0-成功，其它失败
 */
int http_init(char *buff, uint size, p_config_http http, p_config_wifi wifi, p_config_light light);

/**
 * \brief      创建监听socket
 * \param[in]  uint port   端口
 * \return     0-成功，其它失败
 */
int http_create_listen_socket(uint port);

/**
 * \brief      处理客户端的请求
 * \param[in]  int client_sock   客户端socket
 * \return     0-成功，其它失败
 */
int http_process_client_request(int client_sock);

/**
 * \brief      处理客户端的连接
 * \param[in]  int listen_sock   监听socket
 * \return     0-成功，其它失败
 */
int http_process_client_connect(int listen_sock);
