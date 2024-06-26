/**
 *\file     xt_monitor.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.02.08
 *\brief    监控模块定义
 */
#ifndef _XT_MONITOR_H_
#define _XT_MONITOR_H_
#include "xt_list.h"
#include "xt_memory_pool.h"

#ifndef bool
#define bool unsigned char
#endif

#define WHITELIST_SIZE      32                                         ///< 白名单组数大小
#define BLACKLIST_SIZE      32                                         ///< 黑名单组数大小
#define WHITELIST_STR_SIZE  256                                        ///< 白名单正则字符串缓冲区大小
#define BLACKLIST_STR_SIZE  256                                        ///< 黑名单正则字符串缓冲区大小
#define LOCALPATH_SIZE      512                                        ///< 本地监控目录大小
#define REMOTEPATH_SIZE     512                                        ///< 远端服务器目录大小
#define MNT_OBJNAME_SIZE    512                                        ///< 监控事件对象名大小

enum                                                                    ///  监控事件对象类型
{
    EVENT_OBJECT_NULL,                                                  ///< 无
    EVENT_OBJECT_FILE,                                                  ///< 文件
    EVENT_OBJECT_DIR                                                    ///< 目录
};

enum                                                                    ///  监控事件
{
    EVENT_CMD_NULL,                                                     ///< 无
    EVENT_CMD_CREATE,                                                   ///< 新建文件或目录
    EVENT_CMD_DELETE,                                                   ///< 删除文件或目录
    EVENT_CMD_RENAME,                                                   ///< 生命名文件或目录
    EVENT_CMD_MODIFY                                                    ///< 文件内容被修改,重新上传文件
};

typedef struct _xt_monitor                                              ///  监控器
{
    int                 id;                                             ///< 监控ID

    char                localpath[LOCALPATH_SIZE];                      ///< 本地监控目录
    char                remotepath[REMOTEPATH_SIZE];                    ///< 远端服务器目录

    int                 whitelist_count;                                ///< 白名单数量
    int                 blacklist_count;                                ///< 黑名单数量

    char                whitelist[WHITELIST_SIZE][WHITELIST_STR_SIZE];  ///< 白名单
    char                blacklist[BLACKLIST_SIZE][BLACKLIST_STR_SIZE];  ///< 黑名单

    void               *whitelist_pcre[WHITELIST_SIZE][2];              ///< 白名单pcre2数据
    void               *blacklist_pcre[BLACKLIST_SIZE][2];              ///< 黑名单pcre2数据

    bool                run;                                            ///< 监控线程是否运行
    p_xt_list           event;                                          ///< 监控事件列表
    p_xt_memory_pool    pool;                                           ///< 内在池

    int                 ssh_id;                                         ///< SSH的ID

} xt_monitor, *p_xt_monitor;

typedef struct _xt_monitor_event                                        ///  监控事件
{
    char            obj_name[MNT_OBJNAME_SIZE];                         ///< 对象名称,文件或目录名称
    char            obj_oldname[MNT_OBJNAME_SIZE];                      ///< 对象名称,文件或目录名称
    unsigned char   obj_type;                                           ///< 对象类型
    unsigned char   cmd;                                                ///< 监控事件

    unsigned char   monitor_id;                                         ///< 监控ID

} xt_monitor_event, *p_xt_monitor_event;                                

/**
 *\brief                    初始化监控器
 *\param[in]    monitor     监控数据
 *\param[in]    list        监控事件列表
 *\param[in]    pool        内存池
 *\return       0           成功
 */
int monitor_init(p_xt_monitor monitor, p_xt_list list, p_xt_memory_pool pool);

#endif
