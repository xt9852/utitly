/**
 *\file     xt_base64.h
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2012.06.25
 *\brief    BASE64模块定义
 */
#ifndef _XT_BASE64_H_
#define _XT_BASE64_H_

/**
 *\brief                    得到BASE64串
 *\param[in]  data          数据
 *\param[in]  data_len      数据长度
 *\param[out] base64        BASE64字符串
 *\param[out] base64_len    BASE64字符串长度
 *\return     0             成功
 */
int base64_encode(const char *data, int data_len, char *base64, int *base64_len);

/**
 *\brief                    从BASE65串得到原字符串
 *\param[in]  base64        BASE64字符串数据
 *\param[in]  base64_len    BASE64字符串数据长度
 *\param[out] data          输出数据缓冲
 *\param[out] data_len      输出数据缓冲区大小
 *\return     0             成功
 */
int base64_decode(const char *base64, int base64_len, char *data, int *data_len);

#endif
