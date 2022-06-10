/**
 *\copyright    XT Tech. Co., Ltd.
 *\file         xt_base64.c
 *\author       xt
 *\version      1.0.0
 *\date         2012-06-25
 *\brief        BASE64模块实现,UTF-8(No BOM)
 */
#include "xt_base64.h"
#include "xt_log.h"

#ifndef NULL
#define NULL 0
#endif

/// base64字符
const static char BASE64_STRING[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', //   0 -   9
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', //  10 -  19
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', //  20 -  29
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', //  30 -  39
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', //  40 -  49
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', //  50 -  59
	'8', '9', '+', '/'								  //  60 -  63
};

/// 转成base64输入
struct to_base64i
{
	unsigned char i[3];
};

/// 转成base64输出
struct to_base64o
{
	unsigned char o0 : 2;   ///< 2bit
	unsigned char o1 : 6;
	unsigned char o2 : 4;
	unsigned char o3 : 4;
	unsigned char o4 : 6;
	unsigned char o5 : 2;
};

/// 转成base64
typedef union
{
	struct to_base64i i;    ///< 输入
	struct to_base64o o;    ///< 输出

}to_base64;

/// 从base64转回输入
struct from_base64i
{
	unsigned char i[4];
};

/// 从base64转回输出
struct from_base64o
{
	unsigned char o0 : 6;   ///< 6bit
	unsigned char o1 : 2;
	unsigned char o2 : 4;
	unsigned char o3 : 2;
	unsigned char o4 : 2;
	unsigned char o5 : 2;
	unsigned char o6 : 4;
	unsigned char o7 : 2;
	unsigned char o8 : 6;
	unsigned char o9 : 2;
};

/// 从base64转回
typedef union
{
	struct from_base64i i;  ///< 输入
	struct from_base64o o;  ///< 输出

}from_base64;

unsigned char to(struct to_base64o *o, int i)
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

unsigned char from(struct from_base64o *o, int i)
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
 *\brief      得到BASE64串
 *\param[in]  data          数据
 *\param[in]  data_len      数据长度
 *\param[out] base64        BASE64字符串
 *\param[out] base_len      BASE64字符串长度
 *\return     0             成功
 */
int base64_to(const char *data, int data_len, char *base64, int *base_len)
{
    if (NULL == data || data_len <=0 || NULL == base64 || *base_len <= 0)
    {
        return -1;
    }

	int times           = data_len / 3;
	int remain          = data_len % 3;
	int padding         = remain ? (3 - remain) : 0;    // 需要补齐的数量
    int pos_end_data    = times * 3;
	int pos_end_base64  = times * 4;
	int len             = (times + (remain > 0 ? 1 : 0) ) * 4;

    DBG("times:%d remain:%d padding:%d pos_end_data:%d pos_end_base64:%d len:%d",
         times, remain, padding, pos_end_data, pos_end_base64, len);

    if (*base_len <= len)
    {
        return -2;
    }

	to_base64 *item = (to_base64*)data;

	for (int i = 0; i < times; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			base64[i * 4 + j] = BASE64_STRING[to(&(item->o), j)];
		}

		item++;
	}

	to_base64 tail = {0};
	memcpy(&tail, &data[pos_end_data], remain);

	for (int i = 0; i < remain + 1; i++)    // 因为1个字节变成base64时大于1位所以加1
	{
		base64[pos_end_base64 + i] = BASE64_STRING[to(&(tail.o), i)];
	}

	for (int i = 0; i < padding; i++)
	{
		base64[len - padding + i] = 0x3d;   // '='
	}

    base64[len] = '\0';

	*base_len = len;
	return 0;
}

/**
 *\brief      从BASE65串得到原字符串
 *\param[in]  base64        BASE64字符串数据
 *\param[in]  base64_len    BASE64字符串数据长度
 *\param[out] data          输出数据缓冲
 *\param[out] data          输出数据缓冲区大小
 *\return     0             成功
 */
int base64_from(const char *base64, int base64_len, char *data, int *data_len)
{
    if (NULL == base64 || base64_len <= 0 || NULL == data || *data_len <=0)
    {
        return -1;
    }

    int j = 0;
    int completion = 0;
    char *input = (char*)malloc(base64_len);

    for (int i = 0; i < base64_len; i++)
    {
        if (0x0a == base64[i])  // 删除回车
        {
            continue;
        }
        else
        {
            input[j++] = base64[i];
        }
    }

    base64_len = j;

	for (int i = 0; i < base64_len; i++)
	{
		if ((base64[i] >= 'A') && (base64[i] <= 'Z'))
		{
			input[i] -= 'A';
		}
		else if ((base64[i] >= 'a') && (base64[i] <= 'z'))
		{
			input[i] -= 71;
		}
		else if ((base64[i] >= '0') && (base64[i] <= '9'))
		{
			input[i] += 4;
		}
		else if (base64[i] == '+')
		{
			input[i] = 62;
		}
		else if (base64[i] == '/')
		{
			input[i] = 63;
		}
		else if (base64[i] == 0x3d) // '='
		{
			input[i] = 0;
            completion++;
		}
	}

	int times = base64_len / 4;
	from_base64 *item = (from_base64*)input;

	for (int i = 0; i < times; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			data[i*3 + j] = from(&(item->o), j);
		}

		item++;
	}

    free(input);

    *data_len = times * 3 - completion;
    data[*data_len] = '\0';

    DBG("times:%d completion:%d len:%d", times, completion, *data_len);
	return 0;
}