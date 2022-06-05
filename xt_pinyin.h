/*************************************************
 * Copyright:   XT Tech. Co., Ltd.
 * File name:   xt_pinyin.h
 * Author:      xt
 * Version:     1.0.0
 * Date:        2022.06.04
 * Code:        UTF-8(No BOM)
 * Description: 拼音模块接口定义
*************************************************/

#ifndef _XT_PINYIN_H_
#define _XT_PINYIN_H_


/**
 * \brief   从资源中加载拼音数据
 * \param   [in]    char                *res_type   资源类名,"PINYIN"
 * \param   [in]    int                  res_id     资源ID
 * \return  0-成功
 */
int pinyin_init_res(char *res_type, int res_id);

/**
 * \brief   从文件中加载拼音数据
 * \param   [in]  const char            *filename   文件名
 * \param   [out] char                  **data      数据
 * \param   [out] unsigned int          *len        数据长
 * \return  0-成功
 */
int pinyin_init(const char *filename, char **data, unsigned int *len);

/**
 * \brief   将gbk转成拼音
 * \param   [in]        const char      *src        源串
 * \param   [in]        unsigned int     src_len    源串长
 * \param   [out]       short           *dst        目标串
 * \param   [in/out]    unsigned int    *dst_len    目标串最大长,目标串长
 * \return  0-成功
 */
int gbk_pinyin(const unsigned char *src, int src_len, char *dst, int *dst_len);

#endif