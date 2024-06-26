/**
 *\file     xt_character_set.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.06.04
 *\brief    字符集转码定义
 */
#ifndef _XT_CHARACTER_SET_H_
#define _XT_CHARACTER_SET_H_

/**
 *\brief                        将utf8转成unicode
 *\param[in]    src             源串
 *\param[in]    src_len         源串字节长
 *\param[out]   dst             目标串
 *\param[out]   dst_short_len   输入缓冲区short大小,输出目标串short长
 *\return       0               成功
 */
int utf8_unicode(const unsigned char *src, unsigned int src_len, unsigned short *dst, unsigned int *dst_short_len);

/**
 *\brief                        将unicode转成gbk
 *\param[in]    src             源串
 *\param[in]    src_short_len   源串short长
 *\param[out]   dst             目标串
 *\param[out]   dst_len         输入缓冲区字节大小,输出目标串字节长
 *\return       0               成功
 */
int unicode_gbk(const unsigned short *src, unsigned int src_short_len, unsigned char *dst, unsigned int *dst_len);

/**
 *\brief                        将unicode转成utf8
 *\param[in]    src             源串
 *\param[in]    src_short_len   源串short长
 *\param[out]   dst             目标串
 *\param[out]   dst_len         输入缓冲区字节大小,输出目标串字节长
 *\return       0               成功
 */
int unicode_utf8(const unsigned short *src, unsigned int src_short_len, unsigned char *dst, unsigned int *dst_len);

/**
 *\brief                    将gbk转成utf8
 *\param[in]    src         源串
 *\param[in]    src_len     源串字节大长
 *\param[out]   dst         目标串
 *\param[out]   dst_len     输入缓冲区字节大小,输出目标串字节长
 *\return       0           成功
 */
int gbk_utf8(const unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len);

#endif