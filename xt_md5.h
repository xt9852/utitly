/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_md5.h
 *\author       xt
 *\version      1.0.0
 *\date         2022.02.08
 *\brief        MD5模块定义,UTF-8(No BOM)
 */
#ifndef _XT_MD5_H_
#define _XT_MD5_H_

/// md5数据结构
typedef struct _xt_md5
{
    unsigned int    A;      ///< 变量A
    unsigned int    B;      ///< 变量B
    unsigned int    C;      ///< 变量C
    unsigned int    D;      ///< 变量D

} xt_md5, *p_xt_md5;        ///< md5类型

/**
 *\brief        得到MD5数据
 *\param[in]    data        数据
 *\param[in]    data_len    数据长度
 *\param[out]   md5         MD5数据
 *\return       0           成功
 */
int md5_get(const char *data, int data_len, p_xt_md5 md5);

/**
 *\brief        得到MD5字符串
 *\param[in]    data        数据
 *\param[in]    data_len    数据长度
 *\param[out]   md5_str     MD5字符串,大写字母
 *\return       0           成功
 */
int md5_get_str(const char *data, int data_len, char *md5_str);

#endif
