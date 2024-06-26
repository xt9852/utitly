/**
 *\file     xt_http.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022-02-08
 *\brief    HTTP模块实现
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "xt_http.h"

#ifdef XT_LOG
    #include "xt_log.h"
#else
    #include <stdio.h>
    #include <stdlib.h>
#ifdef _WINDOWS
    #define D(...)      printf(__VA_ARGS__)
    #define I(...)      printf(__VA_ARGS__)
    #define W(...)      printf(__VA_ARGS__)
    #define E(...)      printf(__VA_ARGS__)
#else
    #define D(args...)  printf(args)
    #define I(args...)  printf(args)
    #define W(args...)  printf(args)
    #define E(args...)  printf(args)
#endif
#endif

#ifndef true
#define true 1
#endif

#define HTTP_HEAD       "HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %d\n\n" ///< 请求头

#define HTTP_FILE_404   "404"                                                   ///< 404时返回数据

typedef struct _client_thread_param                                             ///  客户端线程参数
{
    int         client_sock;                                                    ///< 客户端socket

    p_xt_http   http;                                                           ///< http数据

} client_thread_param, *p_client_thread_param;

const static char HEX_STR[] = "0123456789ABCDEF";                               ///< 十六进制字符

const static char *g_http_code[] =                                              ///< 状态码
{
    "200 OK",
    "404 Not Found"
};

const static char *g_http_type[] =                                              ///< 页面类型
{
    "text/html",
    "text/xml",
    "image/x-icon",
    "image/gif",
    "image/png",
    "image/jpg",
    "image/jpeg"
};

/**
 *\brief                    十六进制字符转数字
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
        E("ch:0x%02x\n", ch);
        return -1;
    }
}

/**
 *\brief                    URI编码
 *\param[in]    in          原始的数据
 *\param[in]    in_len      原始的数据长度
 *\param[out]   out         编码后数据
 *\param[out]   out_len     输入数据缓冲区大小,输出编码后数据长度
 *\return       0           成功
 */
int uri_encode(const char *in, int in_len, char *out, int *out_len)
{
    if (NULL == in || NULL == out || NULL == out_len || *out_len < in_len * 3)
    {
        E("param null or buff too small\n");
        return -1;
    }

    int j = 0;
    unsigned char hex;

    for (int i = 0; i < in_len; i++)
    {
        if ((in[i] >= '0' && in[i] <= '9') || (in[i] >= 'a' && in[i] <= '9') || (in[i] >= 'A' && in[i] <= 'Z'))
        {
            out[j++] = in[i];
        }
        else
        {
            hex = in[i];
            out[j++] = '%';
            out[j++] = HEX_STR[hex / 16];
            out[j++] = HEX_STR[hex % 16];
        }
    }

    out[j] = '\0';
    *out_len = j;

    return 0;
}

/**
 *\brief                    URI解码
 *\param[in]    in          URI数据
 *\param[in]    in_len      URI数据长度
 *\param[out]   out         原始数据
 *\param[out]   out_len     输入数据缓冲区大小,输出解码后数据长度
 *\return       0           成功
 */
int uri_decode(char *in, int in_len, char *out, int *out_len)
{
    if (NULL == in || NULL == out || NULL == out_len || *out_len < in_len)
    {
        E("param null or buff too small\n");
        return -1;
    }

    int   pos = 0;
    int   len = 0;
    char *beg = in;
    char  hex[2];

    char *token = strtok_s(beg, "%", &beg);

    for (int i = 0; NULL != token; i++)
    {
        if (0 == i)
        {
            pos = len = strlen(token);
            strcpy_s(out, len + 1, token);
        }
        else
        {
            hex[0] = http_to_hex(token[0]);

            if (hex[0] < 0)
            {
                return -2;
            }

            hex[1] = http_to_hex(token[1]);

            if (hex[1] < 0)
            {
                return -3;
            }

            out[pos++] = hex[0] * 16 + hex[1];

            len = strlen(token) - 2;

            strcpy_s(out + pos, len + 1, token + 2);

            pos += len;
        }

        token = strtok_s(NULL, "%", &beg);
    }

    return 0;
}

/**
 *\brief                    得到URI中的参数,/index.html?arg1=1&arg2=2
 *\param[out]   data        URI的参数
 *\return       0           成功
 */
int http_get_arg(p_xt_http_data data)
{
    char *arg = strchr(data->uri, '?');

    if (NULL == arg)
    {
        data->arg_count = 0;
        return 0;
    }

    *arg++ = '\0';  // 后移一位到参数

    int i;
    int len;
    char *key;
    char *value;

    key = strtok_s(arg, "&", &arg);

    for (i = 0; NULL != key; i++)
    {
        value = strchr(key, '=');

        if (NULL == value)
        {
            value = "";
            len = 0;
        }
        else
        {
            *value++ = '\0';
            len = strlen(value);
        }

        if (uri_decode(value, len, value, &len) != 0)   // 使用同一块内存
        {
            return -1;
        }

        data->arg[i].key = key;
        data->arg[i].value = value;
        data->arg[i].value_len = len;

        key = strtok_s(NULL, "&", &arg);

        D("key[%d]:%s len:%u value:%s\n", i, data->arg[i].key, len, value);
    }

    data->arg_count = i;
    return 0;
}

/**
 *\brief                    得到URI
 *\param[in]    buf         数据包
 *\param[in]    buf_size    数据包缓冲区大小
 *\param[out]   data        HTTP数据
 *\return       0           成功
 */
int http_get_uri(char *buf, int buf_size, p_xt_http_data data)
{
    data->uri = buf + 4;

    char *end = strchr(data->uri, ' '); // GET /torrent-list?torrent=*** HTTP/1.1

    if (NULL == end)
    {
        E("dont have ' '\n");
        return -1;
    }

    *end = '\0';
    data->content = end + 1;
    data->len = buf_size - (end - buf);

    return 0;
}

/**
 *\brief                    处理客户端的请求
 *\param[in]    http        http服务数据
 *\param[in]    client_sock 客户端socket
 *\param[in]    buf         数据缓冲区
 *\param[in]    buf_size    数据缓冲区大小
 *\return       0           成功
 */
int http_client_request(p_xt_http http, int client_sock, char *buf, int buf_size)
{
    int len = recv(client_sock, buf, buf_size, 0);

    if (len < 0)
    {
        E("recv failed, errno %d\n", errno);
        return -1;
    }
    else if (len == 0) // Connection closed
    {
        D("connection closed\n");
        return -2;
    }

    buf[len] = 0;

    if (0 != strncmp(buf, "GET", 3))
    {
        E("not GET");
        return 1;
    }

    xt_http_data data = {0};

    int ret = http_get_uri(buf, buf_size, &data);

    if (0 != ret)
    {
        E("http_get_uri fail\n");
        return -3;
    }

    D("sock:%d recv:%d uri:%s\n", client_sock, len, data.uri);

    ret = http_get_arg(&data);

    if (0 != ret)
    {
        E("http_get_arg fail\n");
        return -4;
    }

    ret = http->proc(&data);

    D("callback ret:%d\n", ret);

    if (0 != ret)
    {
        ret = 1;
        data.content = HTTP_FILE_404;
        data.type = HTTP_TYPE_HTML;
        data.len = sizeof(HTTP_FILE_404) - 1;
    }

    char head[128];
    ret = sprintf_s(head, sizeof(head), HTTP_HEAD, g_http_code[ret], g_http_type[data.type], data.len);

    ret = send(client_sock, head, ret, 0);              // 发送头部
    D("send head len:%d\n", ret);

    ret = send(client_sock, data.content, data.len, 0); // 发送内容
    D("send data len:%d\n", ret);

    D("\n%s\n", head);
    return 0;
}

/**
 *\brief                    客户端处理函数
 *\param[in]    param       客户端socket和http服务数据
 *\return                   空
 */
void* http_client_thread(p_client_thread_param param)
{
    D("running...\n");

    int   sock = param->client_sock;
    int   size = 10*1024*1024;
    char *buff = malloc(size);

    while (http_client_request(param->http, sock, buff, size) >= 0);

    D("close client socket %d\n", sock);

    shutdown(sock, 0);
    closesocket(sock);

    D("free param\n");

    free(buff);
    free(param);

    D("exit\n");
    return NULL;
}

/**
 *\brief                    处理客户端的连接
 *\param[in]    http        http服务数据
 *\return       0           成功
 */
int http_server_wait_client_connect(p_xt_http http)
{
    D("accepting...\n");

    int client_sock;
    char addr_str[64];

    if (http->ipv4)
    {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        client_sock = accept(http->listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
    }
    else
    {
        struct sockaddr_in6 client_addr;
        int addr_len = sizeof(client_addr);
        client_sock = accept(http->listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        inet_ntop(AF_INET6, &client_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
    }

    if (client_sock < 0)
    {
        E("accept fail, errno %d\n", errno);
        return -1;
    }

    D("accept client socket:%d ip:%s\n", client_sock, addr_str);

    p_client_thread_param param = (p_client_thread_param)malloc(sizeof(client_thread_param));
    param->client_sock = client_sock;
    param->http = http;

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, http_client_thread, param);

    if (ret != 0)
    {
        E("create thread fail, E:%d\n", ret);
        return -1;
    }

    pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收

    D("create client thread\n");
    return 0;
}

/**
 *\brief                    创建监听socket
 *\param[in]    http        http服务数据
 *\return       0           成功
 */
int http_server_create_listen_socket(p_xt_http http)
{
    int ret = 0;
    WSADATA wsa = {0};
    WSAStartup(MAKEWORD(2, 0), &wsa);

    int listen_sock = socket(http->ipv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);

    if (listen_sock == INVALID_SOCKET)
    {
        E("create socket fail, errno:%d errno:%d\n", listen_sock, GetLastError());
        return -1;
    }

    D("create socket %d\n", listen_sock);

    if (http->ipv4)
    {
        struct sockaddr_in listen_addr;
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(http->port);
        inet_pton(AF_INET, http->ip, &listen_addr.sin_addr);
        ret = bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
    }
    else
    {
        struct sockaddr_in6 listen_addr;
        listen_addr.sin6_family = AF_INET6;
        listen_addr.sin6_port = htons(http->port);
        inet_pton(AF_INET6, http->ip, &listen_addr.sin6_addr);
        ret = bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
    }

    if (ret != 0)
    {
        closesocket(listen_sock);
        E("bind socket fail, errno:%d\n", errno);
        return -2;
    }

    D("bind   socket %s\n", http->ipv4 ? "ipv4" : "ipv6");

    ret = listen(listen_sock, 1);

    if (ret != 0)
    {
        closesocket(listen_sock);
        E("listen socket fail, errno:%d\n", errno);
        return -3;
    }

    http->listen_sock = listen_sock;

    D("listen socket %s:%d\n", http->ip, http->port);
    return 0;
}

/**
 *\brief                    HTTP服务主线程
 *\param[in]    http        http服务数据
 *\return                   空
 */
void* http_server_thread(p_xt_http http)
{
    D("running...\n");

    int ret = http_server_create_listen_socket(http);

    if (0 != ret)
    {
        E("create listen socket error\n");
        E("exit\n");
        return NULL;
    }

    while (http->run)
    {
        http_server_wait_client_connect(http);
    }

    shutdown(http->listen_sock, 0);
    closesocket(http->listen_sock);

    D("exit\n");
    return NULL;
}

/**
 *\brief                    初始化http
 *\param[in]    ip          地址
 *\param[in]    port        监听端口
 *\param[in]    proc        处理请求回调
 *\param[in]    http        服务数据
 *\attention    http        需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 */
int http_init(const char *ip, unsigned short port, XT_HTTP_CALLBACK proc, p_xt_http http)
{
    if (NULL == ip || NULL == http)
    {
        return -1;
    }

    http->run  = true;
    http->port = port;
    http->proc = proc;
    http->ipv4 = (NULL != strchr(ip, '.'));
    strcpy_s(http->ip, sizeof(http->ip), ip);

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, http_server_thread, http);

    if (ret != 0)
    {
        E("create thread fail, E:%d\n", ret);
        return -2;
    }

    pthread_detach(tid);    // 使线程处于分离状态,线程资源由系统回收

    D("ok\n");
    return 0;
}