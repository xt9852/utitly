/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_character_set.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 字符集转码接口定义
*************************************************/

#ifndef _XT_CHARACTER_SET_H_
#define _XT_CHARACTER_SET_H_


/**
 * \brief   将utf8转成unicode
 * \param   [in]        const char      *src        源串
 * \param   [in]        unsigned int     src_len    源串长
 * \param   [out]       short           *dst        目标串
 * \param   [in/out]    unsigned int    *dst_len    目标串最大长,目标串长
 * \return  0-成功
 */
int utf8_unicode(const char *src, int src_len, short *dst, int *dst_len);

/**
 * \brief   将unicode转成ansi(gbk)
 * \param   [in]        const char      *src        源串
 * \param   [in]        unsigned int     src_len    源串长
 * \param   [out]       short           *dst        目标串
 * \param   [in/out]    unsigned int    *dst_len    目标串最大长,目标串长
 * \return  0-成功
 */
int unicode_ansi(const short *src, int src_len, char *dst, int *dst_len);

#endif