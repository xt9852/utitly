/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_notify.c
 *\author       xt
 *\version      1.0.0
 *\date         2022-02-08
 *\brief        系统托盘图标模块实现,UTF-8(No BOM)
 */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "xt_notify.h"
#include "xt_log.h"

NOTIFYICONDATAA     g_notify_data           = {0};      ///< 系统托盘数量
HMENU               g_notify_menu           = NULL;     ///< 菜单
notify_menu_info    g_notify_menu_data[64]  = {0};      ///< 菜单数据
int                 g_notify_menu_count     = 0;        ///< 菜单数量
int                 g_notify_menu_time      = 0;        ///< 菜单定时

/**
 *\brief        定时任务
 *\param[in]    wnd             窗体句柄
 *\param[in]    w               定时器ID
 *\return                       无
 */
void notify_on_timer(HWND wnd, WPARAM w)
{
    D("1 %d", g_notify_menu_time);

    if (g_notify_menu_time-- == 0)
    {
        SendMessage(wnd, WM_CANCELMODE, 0, 0);  // 关闭菜单
    }

    D("2 %d", g_notify_menu_time);
}

/**
 *\brief        系统托盘消息处理函数
 *\param[in]    wnd             窗体句柄
 *\param[in]    l               操作
 *\return                       无
 */
void notify_on_sys_msg(HWND wnd, LPARAM l)
{
    if (LOWORD(l) == WM_RBUTTONDOWN || LOWORD(l) == WM_LBUTTONDOWN)
    {
        g_notify_menu_time = 5;

        POINT pt;
        GetCursorPos(&pt);  // 得到鼠标位置
        TrackPopupMenu(g_notify_menu, 0, pt.x, pt.y, 0, wnd, 0);
    }
}

/**
 *\brief        窗体关闭处理函数 \n
                当用户点击窗体上的关闭按钮时 \n
                系统发出WM_CLOSE消息,自己执行DestroyWindow关闭窗口 \n
                然后发送WM_DESTROY消息,自己执行PostQuitMessage关闭应用程序 \n
                最后发出WM_QUIT消息来关闭消息循环
 *\param[in]    wnd             窗体句柄
 *\return                       无
 */
void notify_on_close(HWND wnd)
{
    DestroyWindow(wnd);
}

/**
 *\brief        窗体消毁处理函数
 *\param[in]    wnd             窗体句柄
 *\return                       无
 */
void notify_on_destory(HWND wnd)
{
    Shell_NotifyIconA(NIM_DELETE, &g_notify_data);
    PostQuitMessage(0);
}

/**
 *\brief        命令消息处理函数,菜单,按钮都会发此消息
 *\param[in]    wnd             窗体句柄
 *\param[in]    w               消息参数
 *\return                       无
 */
void notify_on_command(HWND wnd, WPARAM w)
{
    int id = LOWORD(w);

    for (int i = 0; i < g_notify_menu_count; i++)
    {
        if (id == g_notify_menu_data[i].id)
        {
            g_notify_menu_data[i].proc(wnd, g_notify_menu_data[i].param);
        }
    }
}

/**
 *\brief        窗体类消息处理回调函数
 *\param[in]    wnd             窗体句柄
 *\param[in]    msg             消息ID
 *\param[in]    w               消息参数
 *\param[in]    l               消息参数
 *\return                       消息处理结果,它与发送的消息有关
 */
LRESULT CALLBACK notify_window_msg_callback(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
    D("1 wnd:%x msg:%x", wnd, msg);

    if (msg == g_notify_data.uCallbackMessage)
    {
        notify_on_sys_msg(wnd, l);
    }

    switch(msg)
    {
        case WM_COMMAND:    notify_on_command(wnd, w);  break;
        case WM_TIMER:      notify_on_timer(wnd, w);    break;
        case WM_CLOSE:      notify_on_close(wnd);       return 0;
        case WM_DESTROY:    notify_on_destory(wnd);     return 0;
    }

    D("2 wnd:%x msg:%x", wnd, msg);

    return DefWindowProc(wnd, msg, w, l);
}

/**
 *\brief        设置操作系统任务栏右侧的托盘图标和菜单
 *\param[in]    instance        当前实例句柄
 *\param[in]    icon_id         icon_id图标ID
 *\param[in]    title           窗体标题
 *\param[in]    menu_count      菜单数量
 *\param[in]    menu            菜单数据
 *\return       0               成功
 */
int notify_init(HINSTANCE instance, int icon_id, const char *title, int menu_count, notify_menu_info menu[])
{
    if (NULL == instance || NULL == title)
    {
        E("param is null");
        return -1;
    }

    g_notify_menu       = CreatePopupMenu();
    g_notify_menu_count = menu_count;

    for (int i = 0; i < menu_count; i++)
    {
        g_notify_menu_data[i] = menu[i];
        AppendMenuW(g_notify_menu, MF_STRING, menu[i].id, menu[i].name);
    }

    char classname[512];
    sprintf_s(classname, sizeof(classname), "%s_classname", title);
    D(classname);

    // 窗体类
    WNDCLASSA wc     = {0};
    wc.lpfnWndProc   = notify_window_msg_callback;      // 窗体消息处理函数
    wc.lpszClassName = classname;                       // 类名称

    if (0 == RegisterClassA(&wc))                       // 0-失败
    {
        E("RegisterClass %s fail", classname);
        return -2;
    }

    char windowsname[512];
    sprintf_s(windowsname, sizeof(windowsname), "%s_windowname", title);
    D(windowsname);

    // 创建窗体
    HWND wnd = CreateWindowA(classname,                 // 类名称
                             windowsname,               // 窗体标题
                             WS_OVERLAPPEDWINDOW,       // 窗体属性
                             0, 0,                      // 窗体位置
                             0, 0,                      // 窗体大小
                             NULL,                      // 父窗句柄
                             NULL,                      // 菜单句柄
                             instance,                  // 实例句柄
                             NULL);                     // 参数,给WM_CREATE的lParam

    if (NULL == wnd)
    {
        E("CreateWindow %s fail %d", windowsname, GetLastError());
        return -3;
    }

    char messagename[512];
    sprintf_s(messagename, sizeof(messagename), "%s_messagename", title);
    D(messagename);

    // 系统托盘
    g_notify_data.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;              // 消息,图标,信息
    g_notify_data.hWnd             = wnd;                                           // 指定接收托盘消息的句柄
    g_notify_data.hIcon            = LoadIcon(instance, MAKEINTRESOURCE(icon_id));  // 指定托盘图标
    g_notify_data.uCallbackMessage = RegisterWindowMessageA(messagename);           // 系统托盘消息ID
    strcpy_s(g_notify_data.szTip, sizeof(g_notify_data.szTip), title);              // 信息

    if (!Shell_NotifyIconA(NIM_ADD, &g_notify_data))
    {
        E("Shell_NotifyIcon %s fail", title);
        return -4;
    }

    SetTimer(wnd, 0, 1000, NULL);

    return 0;
}
