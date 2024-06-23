/**
 *\file     xt_exe_ico.c
 *\author   xt
 *\version  1.0.0
 *\date     2022-02-08
 *\brief    EXE程序图标模块实现
 */
#include <windows.h>
#include "xt_exe_ico.h"
#include "xt_log.h"

#define RT_GROUP_ICONA   MAKEINTRESOURCEA((ULONG_PTR)(RT_ICON) + DIFFERENCE)    ///< 程序图标组

#define RT_ICONA         MAKEINTRESOURCEA(3)                                    ///< 程序图标

/**
 *\brief                        得到exe中图标数据
 * ***
 * **图标文件格式**: 图标文件头+图片入口+图片数据
 *
 * **图标文件头**
 * 位置|大小|说明
 * -|-|-
 * 0|2|保留字段,必须为0
 * 2|2|文件类型,必须为1
 * 4|2|图片数量
 *
 * **图片入口** 每一张图片都有一个入口
 * 位置|大小|说明
 * -|-|-
 * 0|1|宽,256为0
 * 1|1|高,256为0
 * 2|1|颜色数
 * 3|1|保留字段,必须为1
 * 4|2|平面数,一般为1
 * 6|2|比特数,4,8,24,32
 * 8|4|本张图片数据长度
 * 12|4|本张图片数据位置
 *
 * ***
 *\param[in]    id              图片在图标中的id
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return                       无
 */
void exe_ico_get_item(unsigned int id, unsigned char *data, unsigned int *data_len)
{
    HRSRC          ico_res  = FindResourceA(NULL, MAKEINTRESOURCEA(id), RT_ICONA);
    HGLOBAL        ico_load = LoadResource(NULL, ico_res);
    unsigned char *ico_data = LockResource(ico_load);
    unsigned int   ico_size = SizeofResource(NULL, ico_res);

    memcpy(data, ico_data, ico_size);

    *data_len = ico_size;
}

/**
 *\brief                        exe中的图标和实际图标有差异,"本张图片数据位置"数据是"1,2,3.."且只有2个字节,需要修改为4个字节
 *\param[in]    in              图标数据
 *\param[in]    in_len          图标数据长度
 *\param[out]   out             修正后图标数据
 *\param[out]   out_len         修正后图标数据长度
 *\return                       无
 */
void exe_ico_update_entry(unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len)
{
    unsigned char  img_count   = in[4];
    unsigned char *p_in_entry  = &in[6];
    unsigned char *p_out_entry = &out[6];

    unsigned int   img_pos = 6 + 16 * img_count; // 图标文件头长度+图片入口长度
    unsigned int   img_len = 0;
    unsigned int   img_total_len = 0;

    D("icon img count:%d", img_count);

    memcpy(out, in, 6); // 图标文件头

    for (unsigned char i = 0; i < img_count; i++) // 图片入口长度
    {
        memcpy(p_out_entry, p_in_entry, 12);

        *(int*)(p_out_entry + 12) = img_pos;

        img_len = *(unsigned int*)(p_in_entry + 8);

        D("img entry id:%d len:%d pos:%d", i, img_len, img_pos);

        img_pos += img_len;
        img_total_len += img_len;

        p_in_entry += 14;
        p_out_entry += 16;
    }

    *out_len = in_len + img_count * 2;

    D("in_len:%d out_len:%d", in_len, *out_len);
}

/**
 *\brief                        得到exe中图标数据
 *\param[in]    ico_id          图标ID
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return       0               成功
 */
int exe_ico_get_data(unsigned int ico_id, unsigned char *data, unsigned int *data_len)
{
    if (NULL == data || NULL == data_len)
    {
        return -1;
    }

    HRSRC group_res = FindResourceA(NULL, MAKEINTRESOURCEA(ico_id), RT_GROUP_ICONA);

    if (NULL == group_res)
    {
        return -2;
    }

    HGLOBAL        group_load = LoadResource(NULL, group_res);
    unsigned char *group_data = LockResource(group_load);
    unsigned int   group_size = SizeofResource(NULL, group_res);

    exe_ico_update_entry(group_data, group_size, data, data_len);

    int pos = *data_len;

    for (int i = 0; i < group_data[4]; i++)
    {
        exe_ico_get_item(i + 1, data + pos, data_len);

        D("get img data id:%d len:%d", i, *data_len);

        pos += *data_len;
    }

    *data_len = pos;

    return 0;
}