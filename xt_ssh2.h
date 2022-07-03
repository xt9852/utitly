/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_ssh2.h
 *\author       xt
 *\version      1.0.0
 *\date         2022.02.08
 *\brief        SSH模块定义,UTF-8(No BOM)
 */
#ifndef _XT_SSH2_
#define _XT_SSH2_
#include "xt_log.h"

#define CMD_SIZE        16                              ///< 命令组大小
#define CMD_STR_SIZE    32                              ///< 命令字符串组大小
#define ADDR_SIZE       128                             ///< 地址字符串组大小
#define USERNAME_SIZE   128                             ///< 用户名称字符串组大小
#define PASSWORD_SIZE   128                             ///< 用户密码串组大小


/// SSH类型
enum
{
    SSH_TYPE_SSH,                                       ///< SSH协议
    SSH_TYPE_SFTP,                                      ///< SFTP协议
};

/// SSH数据回调
typedef int(*XT_SSH_DATA_CALLBACK)(void *param, const char *data, unsigned int data_len);

/// 命令
typedef struct _xt_ssh_cmd
{
    char            str[CMD_STR_SIZE];                  ///< 命令字符串
    unsigned int    sleep;                              ///< 等待时间秒

} xt_ssh_cmd, *p_xt_ssh_cmd;                            ///< 命令指针

/// SSH数据
typedef struct _xt_ssh
{
    int                     type;                       ///< 0-ssh,1-sftp
    bool                    run;                        ///< 是否运行
    bool                    init;                       ///< 是否初始化完成

    unsigned short          port;                       ///< 端口号
    char                    addr[ADDR_SIZE];            ///< 服务器IP地址
    char                    username[USERNAME_SIZE];    ///< 用户名称
    char                    password[PASSWORD_SIZE];    ///< 用户密码

    unsigned short          cmd_count;                  ///< 命令数量
    xt_ssh_cmd              cmd[CMD_SIZE];              ///< 登录服务器时的执行的初始命令

    XT_SSH_DATA_CALLBACK    proc;                       ///< SSH回调
    void                   *param;                      ///< 自定义参数

    time_t                  cmd_time;                   ///< 上一次发送命令时间

    void                   *session;                    ///< SSH的session数据
    void                   *channel;                    ///< SSH的channel数据

} xt_ssh, *p_xt_ssh;                                    ///< SSH数据指针

/**
 *\brief        SSH初始化
 *\param[in]    proc        显示回调
 *\param[in]    param       显示回调自定义参数
 *\param[out]   ssh         SSH数据
 *\return       0           成功
 */
int ssh_init(XT_SSH_DATA_CALLBACK proc, void *param, p_xt_ssh ssh);

/**
 *\brief        SSH初始化
 *\param[in]    addr        服务器地址
 *\param[in]    port        服务器端口
 *\param[in]    username    用户名称
 *\param[in]    password    用户密码
 *\param[in]    cmd_count   初始化命令数量
 *\param[in]    cmd         初始化命令
 *\param[in]    proc        显示回调
 *\param[in]    param       显示回调自定义参数
 *\param[out]   ssh         SSH数据
 *\return       0           成功
 */
int ssh_init_ex(const char *addr, unsigned short port, const char *username, const char *password,
                unsigned short cmd_count, p_xt_ssh_cmd cmd,
                XT_SSH_DATA_CALLBACK proc, void *param, p_xt_ssh ssh);

/**
 *\brief        发送命令并接收数据
 *\param[in]    ssh         SSH参数数据
 *\param[in]    cmd         命令字符串
 *\param[in]    cmd_len     命令字符串长度
 *\param[out]   buf         接收缓冲区
 *\param[out]   buf_size    接收缓冲区大小
 *\return       0           成功
 */
int ssh_send_cmd(p_xt_ssh ssh, const char *cmd, unsigned int cmd_len, char *buf, unsigned int buf_size);

/**
 *\brief        SSH的rz命令上传文件
 *\param[in]    ssh         SSH数据
 *\param[in]    local       本地文件名
 *\param[in]    remote      远端文件名
 *\return       0           成功
 */
int ssh_send_cmd_rz(p_xt_ssh ssh, const char *local, const char *remote);

/**
 *\brief        SSH的sz命令下载文件
 *\param[in]    ssh         SSH数据
 *\param[in]    remote      远端文件名
 *\param[in]    local       本地文件名
 *\return       0           成功
 */
int ssh_send_cmd_sz(p_xt_ssh ssh, const char *remote, const char *local);

#endif
