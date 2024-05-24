/**
 * @file log.h
 * @author YToyCoder(3312034934@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __log__H_
#define __log__H_

#include "Util/buffer/buffer.h"

typedef struct __log 
{
  fixed_buf* pLogCache;
};

int data_to_hex(uint8_t* pData, int nDataLen, fixed_buf* pBuf);

#endif