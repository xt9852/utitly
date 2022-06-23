/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_exe_ico.c
 *\author       xt
 *\version      1.0.0
 *\date         2022-02-08
 *\brief        EXE程序图标模块实现,UTF-8(No BOM)
 */
#include <windows.h>
#include "xt_exe_ico.h"
#include "xt_log.h"

/// 程序图标组
#define RT_GROUP_ICONA   MAKEINTRESOURCEA((ULONG_PTR)(RT_ICON) + DIFFERENCE)

/// 程序图标
#define RT_ICONA         MAKEINTRESOURCEA(3)

/**
 *\brief        得到exe中图标数据
 *\param[in]    id              图片在图标中的id
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return                       无
 */
void exe_ico_get_item(int id, char *data, int *data_len)
{
    HRSRC    ico_res  = FindResourceA(NULL, MAKEINTRESOURCEA(id), RT_ICONA);
    HGLOBAL  ico_load = LoadResource(NULL, ico_res);
    char    *ico_data = LockResource(ico_load);
    int      ico_size = SizeofResource(NULL, ico_res);

    memcpy(data, ico_data, ico_size);

    *data_len = ico_size;
}

/**
 *\brief        exe中的图标和实际图标有差异,这个是通过BeyondCompare比较得到的数据
 *\param[in]    in              图标数据
 *\param[in]    in_len          图标数据长度
 *\param[out]   out             修正后图标数据
 *\param[out]   out_len         修正后图标数据长度
 *\return                       无
 */
void exe_ico_update_data(const char *in, int in_len, char *out, int *out_len)
{
    for (int i = 0, j = 0; i < in_len; i++, j++)
    {
        switch (i)
        {
            case 0x12:
            {
                out[j++] = 0x46;
                out[j++] = 0x00;
                out[j]   = 0x00;
                break;
            }
            case 0x20:
            {
                out[j++] = 0x6E;
                out[j++] = 0x01;
                out[j]   = 0x00;
                break;
            }
            case 0x2E:
            {
                out[j++] = 0xD6;
                out[j++] = 0x06;
                out[j]   = 0x00;
                break;
            }
            case 0x3C:
            {
                out[j++] = 0x3E;
                out[j++] = 0x0A;
                out[j]   = 0x00;
                break;
            }
            default:
            {
                out[j] = in[i];
            }
        }
    }

    *out_len = in_len + 8;
}

/**
 *\brief        得到exe中图标数据
 *\param[in]    ico_id          图标ID
 *\param[out]   data            图标数据
 *\param[out]   data_len        图标数据长
 *\return       0               成功
 */
int exe_ico_get_data(int ico_id, char *data, int *data_len)
{
    if (NULL == data || NULL == data_len)
    {
        return -1;
    }

    HRSRC    group_res  = FindResourceA(NULL, MAKEINTRESOURCEA(ico_id), RT_GROUP_ICONA);

    if (NULL == group_res)
    {
        return -2;
    }

    HGLOBAL  group_load = LoadResource(NULL, group_res);
    char    *group_data = LockResource(group_load);
    int      group_size = SizeofResource(NULL, group_res);

    exe_ico_update_data(group_data, group_size, data, data_len);

    D("icon group len:%d", *data_len);

    int pos = *data_len;

    for (int i = 0; i < group_data[4]; i++)
    {
        exe_ico_get_item(i + 1, data + pos, data_len);

        D("icon[%d] len:%d", i, *data_len);

        pos += *data_len;
    }

    *data_len = pos;

    return 0;
}