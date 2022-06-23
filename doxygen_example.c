/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         doxygen_example.c
 *\author       xt
 *\version      1.0.0
 *\date         2022.06.08
 *\brief        doxygen注释实例,UTF-8(No BOM)
 *\class        类名称
 *\note         注解
 *\attention    注意
 *\warning      警告
 *\exception    异常说明
 *\example      doxygen注释实例 doxygen_example.c
 */
#include <stdio.h>

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

/// 结构体定义
typedef struct _struct_info
{
    char param1[512];               ///< 参数1
    int  param2;                    ///< 参数2

} struct_info, *p_struct_info;      ///< 结构体类型

/// 枚举定义
enum enum_test
{
    ENUM1,                          ///< 枚举1
    ENUM2                           ///< 枚举2
};

int   g_doxygen_1  = 0;             ///< 全局变量1

char *g_doxygen_2  = NULL;          ///< 全局变量2

/**
 *\brief        设置日志文件名
 *\param[in]    param1      参数1
 *\param[out]   param2      参数2
 *\attention    manager     需要转递到线线程中,不要释放此内存,否则会野指针
 *\return       0           成功
 *\retval                   整形
 */
void doxygen_example(int param1, char *param2)
{
    for (int i = 0; i < 10; i++)
    {
        printf("%d %d %s\n", i, param1, param2);
    }

    g_doxygen_1  = 0;
    g_doxygen_2  = NULL;
}
