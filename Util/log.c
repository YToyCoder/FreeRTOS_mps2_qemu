/**
 * @file log.c
 * @author YToyCoder(3312034934@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "log.h"

#define _MAGIC_HEX_ (1 << 4)

static uint8_t hex_arr[16] = {
  '0', '1', '2', '3', 
  '4', '5', '6', '7', 
  '8', '9', 'a', 'b', 
  'c', 'd', 'e', 'f'
};

int data_to_hex(uint8_t* pData, int nDataLen, fixed_buf* pBuf)
{
  for (int i = 0; i < nDataLen; i++) {

    BUF_APPEND_CHAR(pBuf, ' ');
    uint8_t value = pData[i];
    BUF_APPEND_CHAR(pBuf, hex_arr[(value / _MAGIC_HEX_)]);
    BUF_APPEND_CHAR(pBuf, hex_arr[(value % _MAGIC_HEX_)]);
  }

  return 1;
}