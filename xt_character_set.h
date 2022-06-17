/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_character_set.h
 *\author       xt
 *\version      1.0.0
 *\date         2022.06.04
 *\brief        字符集转码定义,UTF-8(No BOM)
 */
#ifndef _XT_CHARACTER_SET_H_
#define _XT_CHARACTER_SET_H_

/**
 *\brief        将utf8转成unicode
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int utf8_unicode(const char *src, int src_len, short *dst, int *dst_len);

/**
 *\brief        将unicode转成ansi(gbk)
 *\param[in]    src         源串
 *\param[in]    src_len     源串长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     目标串最大长,目标串长
 *\return       0           成功
 */
int unicode_ansi(const short *src, int src_len, char *dst, int *dst_len);

#endif