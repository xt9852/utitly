/**
 *\file     xt_uri.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.02.08
 *\brief    URI模块定义
 */
#ifndef _XT_URI_H_
#define _XT_URI_H_

/**
 *\brief                    URI编码
 *\param[in]    in          原始的数据
 *\param[in]    in_len      原始的数据长度
 *\param[out]   out         编码后数据
 *\param[out]   out_len     输入数据缓冲区大小,输出编码后数据长度
 *\return       0           成功
 */
int uri_encode(const char *in, unsigned int in_len, char *out, unsigned int *out_len);

/**
 *\brief        URI解码
 *\param[in]    in          URI数据
 *\param[in]    in_len      URI数据长度
 *\param[out]   out         原始数据
 *\param[out]   out_len     输入数据缓冲区大小,输出解码后数据长度
 *\return       0           成功
 */
int uri_decode(const char *in, unsigned int in_len, char *out, unsigned int *out_len);

#endif
