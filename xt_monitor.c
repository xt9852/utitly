﻿/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_monitor.c
 *\author       xt
 *\version      1.0.0
 *\date         2022.02.08
 *\brief        监控模块实现,UTF-8(No BOM)
 */
#define  PCRE2_STATIC               // 使用静态库
#define  PCRE2_CODE_UNIT_WIDTH 8    // 8位宽
#include "pcre2.h"
#include "xt_monitor.h"
#include "xt_character_set.h"

/**
 *\brief        得到事件对象类型
 *\param[in]    path        路径
 *\param[in]    path_len    路径长
 *\param[in]    path_size   路径缓冲区大小
 *\param[in]    name        文件或目录名称
 *\return                   空
 */
int monitor_get_event_object(char *path, int path_len, int path_size, const char *name)
{
    strncpy_s(&(path[path_len]), path_size - path_len - 1, name, path_size - path_len - 2);

    DWORD attr = GetFileAttributesA(path);  // 得到文件属性

    if (attr == 0xFFFFFFFF)
    {
        return EVENT_OBJECT_NULL; // 无文件
    }

    return (attr & FILE_ATTRIBUTE_DIRECTORY) ? EVENT_OBJECT_DIR : EVENT_OBJECT_FILE;
}

/**
 *\brief        处理白名单
 *\param[in]    monitor     监控数据
 *\param[in]    txt         文件名
 *\return                   空
 */
int monitor_whitelist(p_xt_monitor monitor, const char *txt)
{
    int ret;

    for (int i = 0; i < monitor->whitelist_count; i++)
    {
        ret = pcre2_match(monitor->whitelist_pcre[i][0], txt, strlen(txt), 0, 0, monitor->whitelist_pcre[i][1], NULL);

        if (ret > 0) // <0发生错误，==0没有匹配上，>0返回匹配到的元素数量
        {
            return 0;
        }
    }

    E("whitelist fail %s", txt);
    return -1;
}

/**
 *\brief        处理黑名单
 *\param[in]    monitor     监控数据
 *\param[in]    txt         文件名
 *\return                   空
 */
int monitor_blacklist(p_xt_monitor monitor, const char* txt)
{
    int ret;

    for (int i = 0; i < monitor->blacklist_count; i++)
    {
        ret = pcre2_match(monitor->blacklist_pcre[i][0], txt, strlen(txt), 0, 0, monitor->blacklist_pcre[i][1], NULL);

        if (ret > 0) // <0发生错误，==0没有匹配上，>0返回匹配到的元素数量
        {
            E("blacklist ok %s", txt);
            return -1;
        }
    }

    return 0;
}

/**
 *\brief        处理监控事件
 *\param[in]    monitor     监控数据
 *\param[in]    notify      通知事件
 *\return       0           成功
 */
int monitor_proc_event(p_xt_monitor monitor, FILE_NOTIFY_INFORMATION *notify)
{
    int len;
    int cmd;
    int obj_type;
    int path_len;
    int last_cmd = 0;
    int last_tick = 0;
    char path[MNT_OBJNAME_SIZE];
    char obj_name[MNT_OBJNAME_SIZE];
    char obj_name_last[MNT_OBJNAME_SIZE];
    p_xt_monitor_event event;

    path_len = strlen(monitor->localpath);
    strncpy_s(path, MNT_OBJNAME_SIZE, monitor->localpath, MNT_OBJNAME_SIZE - 1);

    while (monitor->run)
    {
        cmd = EVENT_CMD_NULL;
        len = MNT_OBJNAME_SIZE;
        unicode_utf8(notify->FileName, wcslen(notify->FileName), obj_name, &len);
        obj_type = monitor_get_event_object(path, path_len, MNT_OBJNAME_SIZE, obj_name);

        D("name:%s type:%d", obj_name, obj_type);

        switch (notify->Action)
        {
            case FILE_ACTION_ADDED: // 新建文件或目录
            {
                cmd = EVENT_CMD_CREATE;
                break;
            }
            case FILE_ACTION_REMOVED: // 删除文件或目录
            {
                cmd = EVENT_CMD_DELETE;
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME: // 重命名,旧名
            {
                strncpy_s(obj_name_last, MNT_OBJNAME_SIZE, obj_name, MNT_OBJNAME_SIZE - 1);
                break;
            }
            case FILE_ACTION_RENAMED_NEW_NAME: // 重命名,新名
            {
                cmd = EVENT_CMD_RENAME;
                len = strlen(obj_name);
                obj_name[len] = '|';
                strncpy_s(obj_name + len + 1, MNT_OBJNAME_SIZE, obj_name_last, MNT_OBJNAME_SIZE - len - 2); // 格式:新名|旧名
                break;
            }
            case FILE_ACTION_MODIFIED: // 修改文件或目录,同时可能会有多个修改命令
            {
                if ((obj_type == EVENT_OBJECT_FILE) && (last_cmd != EVENT_CMD_MODIFY || 0 != strcmp(obj_name_last, obj_name) || (GetTickCount() - last_tick) > 500))
                {
                    cmd = EVENT_CMD_MODIFY;
                    strncpy_s(obj_name_last, MNT_OBJNAME_SIZE, obj_name, MNT_OBJNAME_SIZE - 1);
                }
                break;
            }
        }

        if (EVENT_CMD_NULL != cmd && 0 == monitor_whitelist(monitor, obj_name) && 0 == monitor_blacklist(monitor, obj_name))
        {
            last_cmd = cmd;
            last_tick = GetTickCount();
            D("tick:%u obj:%d cmd:%u name:%s", last_tick, obj_type, last_cmd, obj_name);

            memory_pool_get(monitor->pool, &event);
            event->cmd        = cmd;
            event->obj_type   = obj_type;
            event->monitor_id = monitor->id;
            strncpy_s(event->obj_name, MNT_OBJNAME_SIZE, obj_name, MNT_OBJNAME_SIZE - 1);
            list_tail_push(monitor->event, event);
        }

        if (0 == notify->NextEntryOffset)
        {
            break;
        }

        notify = (FILE_NOTIFY_INFORMATION*)((char*)notify + notify->NextEntryOffset);
    }

    return 0;
}

/**
 *\brief        监控器线程
 *\param[in]    monitor     监控数据
 *\return                   空
 */
void* monitor_thread(p_xt_monitor monitor)
{
    D("begin");

    char *path = monitor->localpath;

    // 打开目录,得到目录的句柄
    HANDLE handle = CreateFileA(path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        E("monit %s fail", path);
        return (void*)-1;
    }

    D("monit %s ok", path);

    int ret;
    int len = 10 * 1024 * 1024;
    char *buf = malloc(len);
    FILE_NOTIFY_INFORMATION *notify;

    while (monitor->run)
    {
        notify = (FILE_NOTIFY_INFORMATION*)buf;

        // 设置监控类型,只有用UNICODE类型的函数
        if (!ReadDirectoryChangesW(handle,
            notify,
            len,
            true,
            FILE_NOTIFY_CHANGE_CREATION |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME,
            &ret,
            NULL,
            NULL))
        {
            E("ReadDirectoryChangesW fail,%d", GetLastError());
            Sleep(100);
            continue;
        }

        if (0 == ret)
        {
            E("ReadDirectoryChangesW fail,overflow");
            continue;
        }

        monitor_proc_event(monitor, notify);
    }

    D("exit");
    return NULL;
}

/**
 *\brief        初始化监控器
 *\param[in]    monitor     监控数据
 *\param[in]    event       监控事件列表
 *\param[in]    pool        内存池
 *\return       0           成功
 */
int monitor_init(p_xt_monitor monitor, p_xt_list list, p_xt_memory_pool pool)
{
    if (NULL == monitor)
    {
        return -1;
    }

    int               error;
    PCRE2_SIZE        offset;
    PCRE2_UCHAR       info[512];
    pcre2_code       *pcre_data;
    pcre2_match_data *match_data;

    for (int i = 0; i < monitor->whitelist[i][0] != '\0'; i++)
    {
        pcre_data = pcre2_compile(monitor->whitelist[i], PCRE2_ZERO_TERMINATED, 0, &error, &offset, NULL);

        if (pcre_data == NULL)
        {
            pcre2_get_error_message(error, info, sizeof(info));
            E("pcre2_compile fail reg:%s off:%d info:%s", monitor->whitelist[i], offset, info);
            return -2;
        }

        match_data = pcre2_match_data_create_from_pattern(pcre_data, NULL);

        if (NULL == match_data)
        {
            pcre2_get_error_message(error, info, sizeof(info));
            E("pcre2_match_data_create_from_pattern:fail reg:%s", monitor->whitelist[i]);
            return -3;
        }

        monitor->whitelist_pcre[i][0] = pcre_data;
        monitor->whitelist_pcre[i][1] = match_data;
    }

    for (int i = 0; i < monitor->blacklist[i][0] != '\0'; i++)
    {
        pcre_data = pcre2_compile(monitor->blacklist[i], PCRE2_ZERO_TERMINATED, 0, &error, &offset, NULL);

        if (pcre_data == NULL)
        {
            pcre2_get_error_message(error, info, sizeof(info));
            E("pcre2_compile fail reg:%s off:%d info:%s", monitor->blacklist[i], offset, info);
            return -4;
        }

        match_data = pcre2_match_data_create_from_pattern(pcre_data, NULL);

        if (NULL == match_data)
        {
            pcre2_get_error_message(error, info, sizeof(info));
            E("pcre2_match_data_create_from_pattern:fail reg:%s", monitor->blacklist[i]);
            return -5;
        }

        monitor->blacklist_pcre[i][0] = pcre_data;
        monitor->blacklist_pcre[i][1] = match_data;
    }

    monitor->run = true;
    monitor->pool  = pool;
    monitor->event = list;

    pthread_t tid;

    pthread_create(&tid, NULL, monitor_thread, monitor);

    D("ok");
    return 0;
}