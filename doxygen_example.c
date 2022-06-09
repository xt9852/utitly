/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         doxygen_example.c
 *\author       xt
 *\version      1.0.0
 *\date         2022.06.08
 *\brief        doxygen注释实例,UTF-8(No BOM)
 */

/**
 *\mainpage 此注释能够由doxygen提取, 支持markdown格式
 *
 *\section 标题1
 *
 *此描述在doxygen_example.c中
 *
 *\section 标题2
 *
 * ///  可注释下一行的代码
 *
 * ///< 可注释左侧的代码
 *
 *\section 标题3
 *
 *-# 前面的数字1自动生成
 *-# 前面的数字2自动生成
 *   -# 前面的字母a自动生成
 *   -# 前面的字母b自动生成
 *      -# 前面的罗马数字1自动生成
 *      -# 前面的罗马数字2自动生成
 *          -# 前面的字母A自动生成
 *          -# 前面的字母B自动生成
 *   .
 *   句号为列完结标记
 *
 *\section 标题4
 *
 *Version|Auther|Date|Describe
 *----|----|----|----
 *V1.0.0|xt|2022-06-08|Create File
 *
 */

#include <stdio.h>

/// 结构体定义
typedef struct _struct_info
{
    char param1[512];               ///< 参数1
    int  param2;                    ///< 参数2

} struct_info, *p_struct_info;      ///< 结构体类型

/// 枚举定义
enum
{
    ENUM1,                          ///< 枚举1
    ENUM2                           ///< 枚举2
};

int   g_doxygen_example1  = 0;      ///< 全局变量1

char *g_doxygen_example2  = NULL;   ///< 全局变量1

/**
 *\brief       设置日志文件名
 *\param[in]   param1      参数1
 *\param[out]  param2      参数2
 *\return      0           成功
 */
void doxygen_example(int param1, char *param2)
{
    for (int i = 0; i < 10; i++)
    {
        printf("%d %d %s\n", i, param1, param2);
    }
}
