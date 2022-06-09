/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_base64.h
 *\author       xt
 *\version      1.0.0
 *\date         2012-06-25
 *\brief        BASE64模块接口定义,UTF-8(No BOM)
 */
#ifndef XT_BASE64_H
#define XT_BASE64_H

/**
 *\brief      得到BASE64串
 *\param[in]  data          数据
 *\param[in]  data_len      数据长度
 *\param[out] base64        BASE64字符串
 *\param[out] base_len      BASE64字符串长度
 *\return     0             成功
 */
int base64_to(const char *data, int data_len, char *base64, int *base_len);

/**
 *\brief      从BASE65串得到原字符串
 *\param[in]  base64        BASE64字符串数据
 *\param[in]  base64_len    BASE64字符串数据长度
 *\param[out] data          输出数据缓冲
 *\param[out] data          输出数据缓冲区大小
 *\return     0             成功\n
              <0            失败\n
              >0            原数据长
 */
int base64_from(const char *base64, int base64_len, char *data, int *data_len);

#endif
