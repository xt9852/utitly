/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_http.c
 *\author       xt
 *\version      1.0.0
 *\date         2022-02-08
 *\brief        HTTP模块实现,UTF-8(No BOM)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "xt_http.h"

#define NETWORK_IPV4
#define HTTP_HEAD_200   "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: "
#define HTTP_HEAD_404   "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: "
#define HTTP_FILE_404   "404"

/**
 * \brief      得到URI中的参数,/cpu-data?clk=1 HTTP/1.1
 * \param[in]  uri  URI地址
  * \return         URI中参数指针
 */
char* http_get_arg(char *uri)
{
    char *ch = strchr(uri, ' ');

    if (NULL == ch)
    {
        ERR("request uri:%s error", uri);
        return NULL;
    }

    *ch = '\0';

    DBG("uri:%s", uri);

    ch = strchr(uri, '?');

    if (NULL != ch)
    {
        *ch++ = '\0';
        DBG("arg:%s", ch);
        return ch;
    }

    return NULL;
}

/**
 * \brief      处理客户端的请求
 * \param[in]  http         http服务数据
 * \return     0            成功
 */
int http_client_request(p_xt_http http, char *buff, int buff_size)
{
    DBG("----------------------beg----client_sock:%d", http->client_sock);

    int client_sock = http->client_sock;

    int data_len = recv(client_sock, buff, buff_size, 0);

    if (data_len < 0)
    {
        ERR("recv failed, errno %d", errno);
        return -1;
    }
    else if (data_len == 0) // Connection closed
    {
        ERR("connection closed");
        return -2;
    }
    else
    {
        DBG("sock %d recv data len:%d", client_sock, data_len);

        buff[data_len] = 0;

        if (0 != strncmp(buff, "GET ", 4))
        {
            ERR("request is not GET");
            return 1;
        }

        DBG("\n%s", buff);

        char *uri = buff + 4;
        char *arg = http_get_arg(uri);
        char *head;
        char *content = buff;
        int   head_len;
        int   content_len = buff_size;

        int ret = http->proc(uri, arg, 1, content, &content_len);

        if (ret == 200)
        {
            head = HTTP_HEAD_200;
            head_len = sizeof(HTTP_HEAD_200) - 1;
        }
        else
        {
            head = HTTP_HEAD_404;
            content = HTTP_FILE_404;
            head_len = sizeof(HTTP_HEAD_404) - 1;
            content_len = sizeof(HTTP_FILE_404) - 1;
        }

        DBG("head len:%d content len:%d", head_len, content_len);

        char content_len_str[16];
        sprintf_s(content_len_str, sizeof(content_len_str), "%d\n\n", content_len);     // 内容长度,加2个\n

        ret = send(client_sock, head, head_len, 0);                                     // 发送头部
        DBG("send head len:%d", ret);

        ret = send(client_sock, content_len_str, strlen(content_len_str), 0);           // 发送内容长度
        DBG("send content_len len:%d", ret - 2);

        ret = send(client_sock, content, content_len, 0);                               // 发送内容
        DBG("send content len:%d", ret);

        DBG("\n%s%s", head, content_len_str);
    }

    DBG("----------------------end----");
    return 0;
}

/**
 * \brief      客户端处理函数
 * \param[in]  http         http服务数据
 * \return                  空
 */
void* http_client_thread(p_xt_http http)
{
    DBG("running...");

    int client_sock = http->client_sock;
    int buff_size = 1024*100;
    char *buff = malloc(buff_size);

    while (http_client_request(http, buff, buff_size) >= 0);

    DBG("close client socket %d", client_sock);
    shutdown(client_sock, 0);
    close(client_sock);
    free(buff);

    DBG("exit");
    return NULL;
}

/**
 * \brief      处理客户端的连接
 * \param[in]  http         http服务数据
 * \return     0            成功
 */
int http_server_wait_client_connect(p_xt_http http)
{
    DBG("accepting...");

#ifdef NETWORK_IPV4
    struct sockaddr_in client_addr;
#else
    struct sockaddr_in6 client_addr;
#endif

    int addr_len = sizeof(client_addr);

    int client_sock = accept(http->listen_sock, (struct sockaddr *)&client_addr, &addr_len);

    if (client_sock < 0)
    {
        ERR("accept fail, errno %d", errno);
        return -1;
    }

    http->client_sock = client_sock;

    char addr_str[64];

#ifdef NETWORK_IPV4
    strcpy_s(addr_str, sizeof(addr_str), inet_ntoa(client_addr.sin_addr));
#else
    inet6_ntoa_r(client_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

    DBG("accept client socket:%d addr:%s", client_sock, addr_str);

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, http_client_thread, http);

    if (ret != 0)
    {
        ERR("create thread fail, err:%d\n", ret);
        return -1;
    }

    DBG("create client thread");
    return 0;
}

/**
 * \brief      创建监听socket
 * \param[in]  port         端口
 * \return     0            成功
 */
int http_server_create_listen_socket(unsigned short port)
{
    int addr_family;
    int ip_protocol;
    char addr_str[32];

    WSADATA wsa = {0};
	WSAStartup(MAKEWORD(2, 0), &wsa);

#ifdef NETWORK_IPV4
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(port);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    strcpy_s(addr_str, sizeof(addr_str), inet_ntoa(listen_addr.sin_addr));
#else
    addr_family = AF_INET6;
    ip_protocol = IPPROTO_IPV6;
    struct sockaddr_in6 listen_addr;
    listen_addr.sin6_family = AF_INET6;
    listen_addr.sin6_port = htons(port);
    bzero(&listen_addr.sin6_addr.un, sizeof(listen_addr.sin6_addr.un));
    inet6_ntoa_r(listen_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);

    if (listen_sock == INVALID_SOCKET)
    {
        ERR("create socket fail, errno %d", listen_sock);
        return -1;
    }

    DBG("create listen socket %d ok", listen_sock);

    int ret = bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

    if (ret != 0)
    {
        close(listen_sock);
        ERR("bind socket fail, errno %d", errno);
        return -2;
    }

    DBG("bind socket ok");

    ret = listen(listen_sock, 1);

    if (ret != 0)
    {
        close(listen_sock);
        ERR("listen socket fail, errno %d", errno);
        return -3;
    }

    DBG("listen socket %s:%d", addr_str, port);
    return listen_sock;
}

/**
 * \brief      HTTP服务主线程
 * \param[in]  http         http服务数据
 * \return                  空
 */
void* http_server_thread(p_xt_http http)
{
    DBG("running...");

    int listen_sock = http_server_create_listen_socket(http->port);

    if (listen_sock <= 0)
    {
        ERR("create listen socket error");
        ERR("exit");
        return NULL;
    }

    http->listen_sock = listen_sock;

    while (http->run)
    {
        http_server_wait_client_connect(http);
    }

    shutdown(listen_sock, 0);

    close(listen_sock);

    DBG("exit");
    return NULL;
}

/**
 * \brief      初始化http
 * \param[in]   http    http服务数据,需要port, proc
 * \return      0       成功
 */
int http_init(p_xt_http http)
{
    if (NULL == http)
    {
        return -1;
    }

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, http_server_thread, http);

    if (ret != 0)
    {
        ERR("create thread fail, err:%d\n", ret);
        return -2;
    }

    DBG("ok");
    return 0;
}