/**
 *\file     xt_base64.c
 *\note     UTF-8
 *\author   xt
 *\version  1.0.0
 *\date     2012.06.25
 *\brief    BASE64模块实现
 */
#include "xt_base64.h"
#include "xt_log.h"

/// base64字符
const static char BASE64_STR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/// 转成base64数据,输入3*8=24bit,输出4*6=24bit,内存排列为,o1 o0 o3 o2 o5 o4,o2+o3组成6bit,o2+o5组成6bit
typedef struct _xt_base64_encode
{
    unsigned char o0 : 2;                   ///< 第1块数据2bit
    unsigned char o1 : 6;                   ///< 第2块数据6bit
    unsigned char o2 : 4;                   ///< 第3块数据4bit
    unsigned char o3 : 4;                   ///< 第4块数据4bit
    unsigned char o4 : 6;                   ///< 第5块数据6bit
    unsigned char o5 : 2;                   ///< 第6块数据2bit

} xt_base64_encode, *p_xt_base64_encode;    ///< 转成base64数据指针

/// 从base64转回数据,输入4*6=24bit,输出3*8=24bit,内存排列为,o1 o0 o4 o3 o2 o7 o6 o5 o9 o8,o0+o3组成8bit,o2+o6组成8bit,o5+o8组成8bit
typedef struct _xt_base64_decode
{
    unsigned char o0 : 6;                   ///< 第1块数据6bit
    unsigned char o1 : 2;                   ///< 第2块数据2bit
    unsigned char o2 : 4;                   ///< 第3块数据4bit
    unsigned char o3 : 2;                   ///< 第4块数据2bit
    unsigned char o4 : 2;                   ///< 第5块数据2bit
    unsigned char o5 : 2;                   ///< 第6块数据2bit
    unsigned char o6 : 4;                   ///< 第7块数据4bit
    unsigned char o7 : 2;                   ///< 第8块数据2bit
    unsigned char o8 : 6;                   ///< 第9块数据6bit
    unsigned char o9 : 2;                   ///< 第10块数据2bit

} xt_base64_decode, *p_xt_base64_decode;    ///< 从base64转回数据指针

/**
 *\brief                    转成BASE64数据
 *\param[in]  o             数据
 *\param[in]  i             数据位置
 *\return     0             成功
 */
unsigned char encode(p_xt_base64_encode o, int i)
{
	switch(i)
	{
        case 0: return o->o1;
        case 1: return o->o0 << 4 | o->o3;
        case 2: return o->o2 << 2 | o->o5;
        case 3: return o->o4;
	}

	return 0;
}

/**
 *\brief                    从BASE64数据转成原数据
 *\param[in]  o             数据
 *\param[in]  i             数据位置
 *\return     0             成功
 */
unsigned char decode(p_xt_base64_decode o, int i)
{
	switch(i)
	{
        case 0: return o->o0 << 2 | o->o3;
        case 1: return o->o2 << 4 | o->o6;
        case 2: return o->o5 << 6 | o->o8;
	}

	return 0;
}

/**
 *\brief                    得到BASE64串
 *\param[in]  data          数据
 *\param[in]  data_len      数据长度
 *\param[out] base64        BASE64字符串
 *\param[out] base64_len    BASE64字符串长度
 *\return     0             成功
 */
int base64_encode(const char *data, int data_len, char *base64, int *base64_len)
{
    if (NULL == data || data_len <=0 || NULL == base64 || *base64_len <= 0)
    {
        E("param null or buff too small");
        return -1;
    }

	int times           = data_len / 3;
	int remain          = data_len % 3;
	int padding         = remain ? (3 - remain) : 0;    // 需要补齐的数量
    int pos_end_data    = times * 3;
	int pos_end_base64  = times * 4;
	int len             = (times + (remain > 0 ? 1 : 0) ) * 4;

    if (*base64_len <= len)
    {
        return -2;
    }

	p_xt_base64_encode item = (p_xt_base64_encode)data;

	for (int i = 0; i < times; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			base64[i * 4 + j] = BASE64_STR[encode(item, j)];
		}

		item++;
	}

	xt_base64_encode tail = {0};
	memcpy(&tail, &data[pos_end_data], remain);

	for (int i = 0; i < remain + 1; i++)    // 因为1个字节变成base64时大于1位所以加1
	{
		base64[pos_end_base64 + i] = BASE64_STR[encode(&tail, i)];
	}

	for (int i = 0; i < padding; i++)
	{
		base64[len - padding + i] = 0x3d;   // '='
	}

    base64[len] = '\0';

	*base64_len = len;
	return 0;
}

/**
 *\brief                    从BASE65串得到数据
 *\param[in]  base64        BASE64字符串数据
 *\param[in]  base64_len    BASE64字符串数据长度
 *\param[out] data          输出数据缓冲
 *\param[out] data_len      输出数据缓冲区大小
 *\return     0             成功
 */
int base64_decode(const char *base64, int base64_len, char *data, int *data_len)
{
    if (NULL == base64 || base64_len <= 0 || NULL == data || *data_len <=0)
    {
        E("param null or buff too small");
        return -1;
    }

    int count = 0;
    int padding = 0;
    char *buff = (char*)malloc(base64_len);

    // 将字符串转成6bit
	for (int i = 0; i < base64_len; i++)
	{
		if ((base64[i] >= 'A') && (base64[i] <= 'Z'))
		{
			buff[count++] = base64[i] - 'A';
		}
		else if ((base64[i] >= 'a') && (base64[i] <= 'z'))
		{
			buff[count++] = base64[i] - 71;
		}
		else if ((base64[i] >= '0') && (base64[i] <= '9'))
		{
			buff[count++] = base64[i] + 4;
		}
		else if (base64[i] == '+')
		{
			buff[count++] = 62;
		}
		else if (base64[i] == '/')
		{
			buff[count++] = 63;
		}
		else if (base64[i] == '=')
		{
            padding++;
			buff[count++] = 0;
		}
	}

	int times = count / 4;
	p_xt_base64_decode item = (p_xt_base64_decode)buff;

	for (int i = 0; i < times; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			data[i*3 + j] = decode(item, j);
		}

		item++;
	}

    free(buff);

    *data_len = times * 3 - padding;
    data[*data_len] = '\0';

    //D("times:%d padding:%d len:%d", times, padding, *data_len);
	return 0;
}