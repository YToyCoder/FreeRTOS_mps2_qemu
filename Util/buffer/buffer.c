/**
 * @file buffer.c
 * @author YToyCoder(3312034934@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "buffer.h"

#include <stdlib.h>
#include "FreeRTOS.h"
#include "portable.h"
#include <string.h>

enum E_Buf_T
{
  E_Buf_T_UserManage,
  E_Buf_T_ToFree
};

typedef struct __internal_fixed_buf
{
  BUFFER_MEMER_DECL()
  int      nBufSize;
  enum E_Buf_T eBufFlag;
} internal_fixed_buf_t;

fixed_buf* buf_init_buf(int bufLen)
{
  internal_fixed_buf_t* pBuf = pvPortMalloc(sizeof(internal_fixed_buf_t));
  if (NULL == pBuf) 
    return NULL;

  pBuf->pData = pvPortMalloc(bufLen);
  if (NULL == pBuf->pData) {
    vPortFree(pBuf);
    return NULL;
  }

  pBuf->nDataLen = 0;
  pBuf->nBufSize = bufLen;
  pBuf->eBufFlag = E_Buf_T_ToFree; 

  return pBuf;
}

fixed_buf* buf_init_buf2(uint8_t* pBuf, int nBufLen)
{
  if (NULL == pBuf)
    return NULL;

  internal_fixed_buf_t* iBuf= pvPortMalloc(sizeof(internal_fixed_buf_t));
  if (NULL == iBuf) 
    return NULL;

  iBuf->pData = pBuf;
  iBuf->nBufSize = nBufLen;
  iBuf->nDataLen = 0;
  iBuf->eBufFlag = E_Buf_T_UserManage;

  return iBuf;
}

int buf_append_data(fixed_buf* pBuf, uint8_t* pData, int nDataLen)
{
  if (NULL == pBuf)
    return 0;

  if (NULL == pData)
    return 0;

  if (pBuf->nDataLen + nDataLen > buf_get_buf_size(pBuf))
    return 0;

  memcpy(pBuf->pData + pBuf->nDataLen, pData, nDataLen);
  pBuf->nDataLen += nDataLen;

  return 1;
}

int buf_append_char(fixed_buf* pBuf, uint8_t ucByte)
{
  if (NULL == pBuf)
    return 0;

  if (pBuf->nDataLen + 1 > buf_get_buf_size(pBuf))
    return 0;

  pBuf->pData[pBuf->nDataLen] = ucByte;
  pBuf->nDataLen += 1;

  return 1;
}

int buf_get_buf_size(fixed_buf* pBuf)
{
  if (NULL == pBuf)
    return 0;

  internal_fixed_buf_t* iBuf = (internal_fixed_buf_t*) pBuf;
  return iBuf->nBufSize;
}

void buf_free_buf(fixed_buf* pBuf)
{
  if (NULL == pBuf)
    return;

  if (NULL == pBuf->pData)
    return;

  internal_fixed_buf_t* iBuf = (internal_fixed_buf_t*) pBuf;

  if (iBuf->eBufFlag == E_Buf_T_ToFree) {
    vPortFree(pBuf->pData);
    iBuf->nBufSize = 0;
    iBuf->nDataLen = 0;
    iBuf->pData = NULL;
  }

  vPortFree(iBuf);
}