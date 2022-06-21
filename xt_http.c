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

/// 使用IPV4
#define NETWORK_IPV4

/// 请求头
#define HTTP_HEAD       "HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %d\n\n"

/// 404时返回数据
#define HTTP_FILE_404   "404"

/// 客户端线程参数
typedef struct _client_thread_param
{
    int         client_sock;    ///< 客户端socket

    p_xt_http   http;           ///< http数据

} client_thread_param, *p_client_thread_param;  ///< 客户端线程参数指针

const static char *g_http_code[] = { "200 OK", "404 Not Found" };   ///< 状态码

const static char *g_http_type[] = { "text/html", "image/x-icon", "image/gif", "image/png", "image/jpg", "image/jpeg" }; ///< 页面类型

/**
 *\brief        十六进制字符转数字
 *\param[in]    ch          十六进制字符
 *\return                   数字
 */
char http_to_hex(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    else
    {
        ERR("ch:0x%02x", ch);
        return -1;
    }
}

/**
 *\brief        转成参数,%3A%2F%2F
 *\param[out]   arg         转成参数
 *\return       0           成功
 */
int http_proc_arg(char *arg)
{
    int   pos = 0;
    int   len = 0;
    char *beg = arg;
    char  hex[2];

    char *token = strtok_s(beg, "%", &beg);

    for (int i = 0; NULL != token; i++)
    {
        if (0 == i)
        {
            pos = len = strlen(token);
            strcpy_s(arg, len + 1, token);
        }
        else
        {
            hex[0] = http_to_hex(token[0]);

            if (hex[0] < 0)
            {
                return -1;
            }

            hex[1] = http_to_hex(token[1]);

            if (hex[1] < 0)
            {
                return -2;
            }

            arg[pos++] = hex[0] * 16 + hex[1];

            len = strlen(token) - 2;

            strcpy_s(arg + pos, len + 1, token + 2);

            pos += len;
        }

        token = strtok_s(NULL, "%", &beg);
    }

    return 0;
}

/**
 *\brief        得到URI中的参数,/index.html?arg1=1&arg2=2
 *\param[in]    uri         URI地址
 *\param[out]   arg         URI的参数
 *\return       0           成功
 */
int http_get_arg(char *uri, p_xt_http_arg arg)
{
    char *beg = strchr(uri, '?');

    if (NULL == beg)
    {
        arg->count = 0;
        return 0;
    }

    *beg++ = '\0';  // 后移一位到参数

    int i;
    char *data;

    char *token = strtok_s(beg, "&", &beg);

    for (i = 0; NULL != token; i++)
    {
        arg->name[i] = token;

        char *equ = strchr(token, '=');

        if (NULL == equ)
        {
            data = "";
        }
        else
        {
            *equ++ = '\0';
            data = equ;
        }

        if (http_proc_arg(data) != 0)
        {
            return -1;
        }

        arg->data[i] = data;

        DBG("arg[%d] name:%s data:%s", i, arg->name[i], data);
        token = strtok_s(NULL, "&", &beg);
    }

    arg->count = i;
    return 0;
}

/**
 *\brief        得到URI
 *\param[in]    buff        数据包
 *\param[out]   uri         URI地址
 *\param[in]    uri_size    URI地址缓冲区大小
 *\return       0           成功
 */
int http_get_uri(const char *buff, char *uri, int uri_size)
{
    char *end = strchr(buff + 4, ' ');

    if (NULL == end)
    {
        ERR("request uri:%s error", uri);
        return -1;
    }

    strncpy_s(uri, uri_size, buff + 4, end - buff - 4);

    return 0;
}

/**
 *\brief        处理客户端的请求
 *\param[in]    http            http服务数据
 *\param[in]    client_sock     客户端socket
 *\param[in]    buff            数据缓冲区
 *\param[in]    buff_size       数据缓冲区大小
 *\return       0               成功
 */
int http_client_request(p_xt_http http, int client_sock, char *buff, int buff_size)
{
    int data_len = recv(client_sock, buff, buff_size, 0);

    if (data_len < 0)
    {
        ERR("recv failed, errno %d", errno);
        return -1;
    }
    else if (data_len == 0) // Connection closed
    {
        DBG("connection closed");
        return -2;
    }

    buff[data_len] = 0;

    if (0 != strncmp(buff, "GET", 3))
    {
        ERR("request is not GET");
        return 1;
    }

    char            uri[1024];
    xt_http_arg     arg         = {0};
    xt_http_content content     = {HTTP_TYPE_HTML, buff_size, buff};

    int ret = http_get_uri(buff, uri, sizeof(uri));

    if (0 != ret)
    {
        return -3;
    }

    DBG("sock:%d recv:%d uri:%s", client_sock, data_len, uri);

    ret = http_get_arg(uri, &arg);

    if (0 != ret)
    {
        return -4;
    }

    ret = http->proc(uri, &arg, &content);

    DBG("call http callback end ret:%d", ret);

    if (0 != ret)
    {
        ret = 1;
        content.data = HTTP_FILE_404;
        content.type = HTTP_TYPE_HTML;
        content.len = sizeof(HTTP_FILE_404) - 1;
    }

    char head[128];
    ret = sprintf_s(head, sizeof(head), HTTP_HEAD, g_http_code[ret], g_http_type[content.type], content.len);

    ret = send(client_sock, head, ret, 0);                  // 发送头部
    DBG("send head len:%d", ret);

    ret = send(client_sock, content.data, content.len, 0);  // 发送内容
    DBG("send content len:%d", ret);

    //DBG("\n%s", head);
    return 0;
}

/**
 *\brief        客户端处理函数
 *\param[in]    param           客户端socket和http服务数据
 *\return                       空
 */
void* http_client_thread(p_client_thread_param param)
{
    DBG("running...");

    int client_sock = param->client_sock;
    int buff_size = 10*1024*1024;
    char *buff = malloc(buff_size);

    while (http_client_request(param->http, client_sock, buff, buff_size) >= 0);

    DBG("close client socket %d", client_sock);

    shutdown(client_sock, 0);
    close(client_sock);

    free(buff);
    free(param);

    DBG("exit");
    return NULL;
}

/**
 *\brief        处理客户端的连接
 *\param[in]    http            http服务数据
 *\return       0               成功
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

    p_client_thread_param param = (p_client_thread_param)malloc(sizeof(client_thread_param));
    param->client_sock = client_sock;
    param->http = http;

    char addr_str[64];

#ifdef NETWORK_IPV4
    strcpy_s(addr_str, sizeof(addr_str), inet_ntoa(client_addr.sin_addr));
#else
    inet6_ntoa_r(client_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

    DBG("accept client socket:%d addr:%s", client_sock, addr_str);

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, http_client_thread, param);

    if (ret != 0)
    {
        ERR("create thread fail, err:%d\n", ret);
        return -1;
    }

    DBG("create client thread");
    return 0;
}

/**
 *\brief        创建监听socket
 *\param[in]    port            端口
 *\return       0               成功
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
 *\brief        HTTP服务主线程
 *\param[in]    http            http服务数据
 *\return                       空
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
 *\brief        初始化http
 *\param[in]    http            服务数据,需要run, port, proc
 *\attention    http            需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0               成功
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