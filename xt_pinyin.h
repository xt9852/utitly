/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_pinyin.h
 *\author       xt
 *\version      1.0.0
 *\date         2022.02.08
 *\brief        拼音模块定义,UTF-8(No BOM)
 */
#ifndef _XT_PINYIN_H_
#define _XT_PINYIN_H_


/**
 * \brief       从资源中加载拼音数据
 * \param[in]   res_type    资源类名,"PINYIN"
 * \param[in]   res_id      资源ID
 * \return      0           成功
 */
int pinyin_init_res(char *res_type, int res_id);

/**
 * \brief       从文件中加载拼音数据
 * \param[in]   filename    文件名
 * \param[out]  *data       数据
 * \param[out]  len         数据长
 * \return      0           成功
 */
int pinyin_init(const char *filename, char **data, unsigned int *len);

/**
 * \brief       将gbk转成拼音
 * \param[in]   src         源串
 * \param[in]   src_len     源串长
 * \param[out]  dst         目标串
 * \param[out]  dst_len     目标串最大长,目标串长
 * \return      0           成功
 */
int gbk_pinyin(const unsigned char *src, int src_len, char *dst, int *dst_len);

#endif