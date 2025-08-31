/**
 *\file     xt_notify.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022-02-08
 *\brief    系统托盘图标模块定义
 */
#ifndef _XT_SYS_ICON_H_
#define _XT_SYS_ICON_H_
#include <windows.h>

typedef void (*XT_NOTIFY_MENU_CALLBACK)(HWND wnd, void* param); ///< 托盘菜单回调接口

typedef struct _notify_menu_info                                ///  系统托盘数据
{
    short                   name[64];                           ///< 菜单名称
    void                   *param;                              ///< 自定义参数
    XT_NOTIFY_MENU_CALLBACK proc;                               ///< 菜单回调

} notify_menu_info, *p_notify_menu_info;

/**
 *\brief                        设置操作系统任务栏右侧的托盘图标和菜单
 *\param[in]    instance        当前实例句柄
 *\param[in]    icon_id         icon_id图标ID
 *\param[in]    title           窗体标题,只能用常量字符串
 *\param[in]    menu_count      菜单数量
 *\param[in]    menu            菜单数据
 *\return       0               成功
 */
int notify_init(HINSTANCE instance, int icon_id, const char *title, unsigned int menu_count, notify_menu_info menu[]);

/**
 *\brief                        windows消息循环
 *\return       0               成功
 */
int notify_loop_msg();

#endif
