/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_exe_icon.h
 *\author       xt
 *\version      1.0.0
 *\date         2022-02-08
 *\brief        EXE程序图标模块实现,UTF-8(No BOM)
 */
#ifndef _XT_EXE_ICON_H_
#define _XT_EXE_ICON_H_


/**
 *\brief        得到exe中图标数据
 *\param[in]    icon_id         图标ID
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return       0               成功
 */
int exe_icon_get_data(int icon_id, char *data, int *data_len);

#endif
