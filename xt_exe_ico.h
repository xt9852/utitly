/**
 *\file     xt_exe_ico.h
 *\author   xt
 *\version  1.0.0
 *\date     2022-02-08
 *\brief    EXE程序图标模块实现
 */
#ifndef _XT_EXE_ICON_H_
#define _XT_EXE_ICON_H_

/**
 *\brief                        得到exe中图标数据
 *\param[in]    ico_id          图标ID
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return       0               成功
 */
int exe_ico_get_data(unsigned int ico_id, unsigned char *data, unsigned int *data_len);

#endif
