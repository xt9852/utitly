/**
 *\file     xt_md5.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2022.02.08
 *\brief    MD5模块实现
 */
#include "xt_md5.h"
#include "xt_log.h"

/*
MD5码以512位分组来处理输入的信息,且每一分组又被划分为16个32位子分组

1.填充
    1) 在信息的后面填充一个1和无数个0,直到满足上面的条件时才停止用0对信息的填充
    2) 在这个结果后面附加一个以64位二进制表示的填充前信息长度(单位为Bit),如果二进制表示的填充前信息长度超过64位,则取低64位

2.初始化变量
    A=0x01234567,B=0x89ABCDEF,C=0xFEDCBA98,D=0x76543210(大端字节序)

3.处理分组数据
    主循环有四轮,
    每轮进行16次操作,每次操作对a、b、c、d中的其中三个作一次非线性函数运算,
    然后将所得结果加上第四个变量,文本的一个子分组和一个常数.
    再将所得结果循环左移一个不定的数,并加上a、b、c、d中之一.最后用该结果取代a、b、c、d中之一

    F(X, Y ,Z) = (X & Y) | ((~X) & Z)
    G(X, Y ,Z) = (X & Z) | (Y & (~Z))
    H(X, Y ,Z) = X ^ Y ^ Z
    I(X, Y ,Z) = Y ^ (X | (~Z))

    FF(a, b, c, d, x, s, co) 为 a = b + ( (a + F(b,c,d) + x + co) << s)
    GG(a, b, c, d, x, s, co) 为 a = b + ( (a + G(b,c,d) + x + co) << s)
    HH(a, b, c, d, x, s, co) 为 a = b + ( (a + H(b,c,d) + x + co) << s)
    II(a, b, c, d, x, s, co) 为 a = b + ( (a + I(b,c,d) + x + co) << s)

    常数ti是4294967296*abs(sin(i))的整数部分,i取值从1到64,单位是弧度
    常数s是移位位数, 第一轮: 7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22
                     第二轮: 5, 9,14,20,5, 9,14,20,5, 9,14,20,5, 9,14,20
                     第三轮: 4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23
                     第四轮: 6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21

*/

/// 非线性函数,攻击者试图逆向MD5时需要解这个非线性函数F,但是解这个非线性函数F不能获得唯一解
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

/// 循环左移
#define MD5_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/// 核心公式
#define MD5_FF(a, b, c, d, x, s, co) { (a) = (b) + MD5_ROTATE_LEFT((a) + MD5_F((b),(c),(d)) + (x) + (unsigned int)(co), (s)); }
#define MD5_GG(a, b, c, d, x, s, co) { (a) = (b) + MD5_ROTATE_LEFT((a) + MD5_G((b),(c),(d)) + (x) + (unsigned int)(co), (s)); }
#define MD5_HH(a, b, c, d, x, s, co) { (a) = (b) + MD5_ROTATE_LEFT((a) + MD5_H((b),(c),(d)) + (x) + (unsigned int)(co), (s)); }
#define MD5_II(a, b, c, d, x, s, co) { (a) = (b) + MD5_ROTATE_LEFT((a) + MD5_I((b),(c),(d)) + (x) + (unsigned int)(co), (s)); }

/// 第一轮
#define MD5_R1 MD5_FF(a, b, c, d, x[ 0],  7, 0xd76aa478);\
               MD5_FF(d, a, b, c, x[ 1], 12, 0xe8c7b756);\
               MD5_FF(c, d, a, b, x[ 2], 17, 0x242070db);\
               MD5_FF(b, c, d, a, x[ 3], 22, 0xc1bdceee);\
               MD5_FF(a, b, c, d, x[ 4],  7, 0xf57c0faf);\
               MD5_FF(d, a, b, c, x[ 5], 12, 0x4787c62a);\
               MD5_FF(c, d, a, b, x[ 6], 17, 0xa8304613);\
               MD5_FF(b, c, d, a, x[ 7], 22, 0xfd469501);\
               MD5_FF(a, b, c, d, x[ 8],  7, 0x698098d8);\
               MD5_FF(d, a, b, c, x[ 9], 12, 0x8b44f7af);\
               MD5_FF(c, d, a, b, x[10], 17, 0xffff5bb1);\
               MD5_FF(b, c, d, a, x[11], 22, 0x895cd7be);\
               MD5_FF(a, b, c, d, x[12],  7, 0x6b901122);\
               MD5_FF(d, a, b, c, x[13], 12, 0xfd987193);\
               MD5_FF(c, d, a, b, x[14], 17, 0xa679438e);\
               MD5_FF(b, c, d, a, x[15], 22, 0x49b40821);

/// 第二轮
#define MD5_R2 MD5_GG(a, b, c, d, x[ 1],  5, 0xf61e2562);\
               MD5_GG(d, a, b, c, x[ 6],  9, 0xc040b340);\
               MD5_GG(c, d, a, b, x[11], 14, 0x265e5a51);\
               MD5_GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa);\
               MD5_GG(a, b, c, d, x[ 5],  5, 0xd62f105d);\
               MD5_GG(d, a, b, c, x[10],  9, 0x02441453);\
               MD5_GG(c, d, a, b, x[15], 14, 0xd8a1e681);\
               MD5_GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8);\
               MD5_GG(a, b, c, d, x[ 9],  5, 0x21e1cde6);\
               MD5_GG(d, a, b, c, x[14],  9, 0xc33707d6);\
               MD5_GG(c, d, a, b, x[ 3], 14, 0xf4d50d87);\
               MD5_GG(b, c, d, a, x[ 8], 20, 0x455a14ed);\
               MD5_GG(a, b, c, d, x[13],  5, 0xa9e3e905);\
               MD5_GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8);\
               MD5_GG(c, d, a, b, x[ 7], 14, 0x676f02d9);\
               MD5_GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);

/// 第三轮
#define MD5_R3 MD5_HH(a, b, c, d, x[ 5],  4, 0xfffa3942);\
               MD5_HH(d, a, b, c, x[ 8], 11, 0x8771f681);\
               MD5_HH(c, d, a, b, x[11], 16, 0x6d9d6122);\
               MD5_HH(b, c, d, a, x[14], 23, 0xfde5380c);\
               MD5_HH(a, b, c, d, x[ 1],  4, 0xa4beea44);\
               MD5_HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9);\
               MD5_HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60);\
               MD5_HH(b, c, d, a, x[10], 23, 0xbebfbc70);\
               MD5_HH(a, b, c, d, x[13],  4, 0x289b7ec6);\
               MD5_HH(d, a, b, c, x[ 0], 11, 0xeaa127fa);\
               MD5_HH(c, d, a, b, x[ 3], 16, 0xd4ef3085);\
               MD5_HH(b, c, d, a, x[ 6], 23, 0x04881d05);\
               MD5_HH(a, b, c, d, x[ 9],  4, 0xd9d4d039);\
               MD5_HH(d, a, b, c, x[12], 11, 0xe6db99e5);\
               MD5_HH(c, d, a, b, x[15], 16, 0x1fa27cf8);\
               MD5_HH(b, c, d, a, x[ 2], 23, 0xc4ac5665);

/// 第四轮
#define MD5_R4 MD5_II(a, b, c, d, x[ 0],  6, 0xf4292244);\
               MD5_II(d, a, b, c, x[ 7], 10, 0x432aff97);\
               MD5_II(c, d, a, b, x[14], 15, 0xab9423a7);\
               MD5_II(b, c, d, a, x[ 5], 21, 0xfc93a039);\
               MD5_II(a, b, c, d, x[12],  6, 0x655b59c3);\
               MD5_II(d, a, b, c, x[ 3], 10, 0x8f0ccc92);\
               MD5_II(c, d, a, b, x[10], 15, 0xffeff47d);\
               MD5_II(b, c, d, a, x[ 1], 21, 0x85845dd1);\
               MD5_II(a, b, c, d, x[ 8],  6, 0x6fa87e4f);\
               MD5_II(d, a, b, c, x[15], 10, 0xfe2ce6e0);\
               MD5_II(c, d, a, b, x[ 6], 15, 0xa3014314);\
               MD5_II(b, c, d, a, x[13], 21, 0x4e0811a1);\
               MD5_II(a, b, c, d, x[ 4],  6, 0xf7537e82);\
               MD5_II(d, a, b, c, x[11], 10, 0xbd3af235);\
               MD5_II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb);\
               MD5_II(b, c, d, a, x[ 9], 21, 0xeb86d391);

/// 主处理
#define MD5_P(X) x=X; a = A; b = B; c = C; d = D; MD5_R1; MD5_R2; MD5_R3; MD5_R4; A += a; B += b; C += c; D += d;

/// MD5字符串
const char *MD5_STRING = "0123456789ABCDEF";

/**
 *\brief                    得到MD5数据
 *\param[in]    data        数据
 *\param[in]    data_len    数据长度
 *\param[out]   md5         MD5数据
 *\return       0           成功
 */
int md5_get(const char *data, int data_len, p_xt_md5 md5)
{
    if (NULL == data || data_len <= 0 || NULL == md5)
    {
        return -1;
    }

    unsigned int     a;
    unsigned int     b;
    unsigned int     c;
    unsigned int     d;
    unsigned int    *x;
    unsigned int     A         = 0x67452301;
    unsigned int     B         = 0xefcdab89;
    unsigned int     C         = 0x98badcfe;
    unsigned int     D         = 0x10325476;
    unsigned int     times     = data_len / 64;
    unsigned int     remain    = data_len % 64;
    unsigned int     padding   = (remain > 55) ? (128 - remain - 1 - 8) : (64 - 1 - 8); // 1个字节为0x80,8个字节为数据比特长
    unsigned int     count_pos = (remain > 55) ? (128 - 8) : (64 - 8);                  // 比特长所在位置
    unsigned __int64 bit_count = data_len * 8;
    unsigned char    buff[128];

    D("md5 times:%d remain:%d padding:%d", times, remain, padding);

    for (unsigned int i = 0; i < times; i++)
    {
        MD5_P((unsigned int *)&data[i * 64]);
    }

    memcpy(buff, &data[times * 64], remain);    // 剩余数据
    buff[remain] = 0x80;                        // 补齐1
    memset(&buff[remain + 1], 0, padding);      // 补齐0
    memcpy(&buff[count_pos], &bit_count, 8);    // 数据比特长

    MD5_P((unsigned int *)buff);

    if (remain > 55)
    {
        MD5_P((unsigned int *)(buff + 64));
    }

    md5->A = A;
    md5->B = B;
    md5->C = C;
    md5->D = D;
    return 0;
}

/**
 *\brief                    得到MD5字符串
 *\param[in]    data        数据
 *\param[in]    data_len    数据长度
 *\param[out]   md5_str     MD5字符串,大写字母
 *\return       0           成功
 */
int md5_get_str(const char *data, int data_len, char *md5_str)
{
    if (NULL == data || data_len <= 0 || NULL == md5_str)
    {
        return -1;
    }

    xt_md5 md5;

    int ret = md5_get(data, data_len, &md5);

    if (0 != ret)
    {
        return ret;
    }

    unsigned char *p = (char*)&md5;

    D("A:%x B:%x C:%x D:%x", md5.A, md5.B, md5.C, md5.D);

    for (int i = 0; i < 16; i++)
    {
        md5_str[2 * i]     = MD5_STRING[p[i] >> 4];
        md5_str[2 * i + 1] = MD5_STRING[p[i] & 0x0F];
    }

    md5_str[32] = '\0';
    return 0;
}
