/**
 * @file buffer.h
 * @author YToyCoder (3312034934@qq.com)
 * @brief simple buffer
 * @version 0.1
 * @date 2024-05-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __buffer__H_
#define __buffer__H_

#include <stdint.h>

#define BUFFER_MEMER_DECL() \
    uint8_t* pData; \
    int      nDataLen; \

struct __fixed_buf {
  BUFFER_MEMER_DECL()
};

typedef struct __fixed_buf fixed_buf;

/**
 * @brief init buffer to fixed len
 * 
 * @param bufLen 
 * @return fixed_buf* 
 */
fixed_buf* buf_init_buf(int bufLen);

/**
 * @brief init buffer from exists buf
 * 
 * @param pBuf 
 * @param nBufLen 
 * @return fixed_buf* 
 */
fixed_buf* buf_init_buf2(uint8_t* pBuf, int nBufLen);

/**
 * @brief 
 * 
 * @param pBuf 
 * @param pData 
 * @param nDataLen 
 * @return int 
 */
int buf_append_data(fixed_buf* pBuf, uint8_t* pData, int nDataLen);

int buf_append_char(fixed_buf* pBuf, uint8_t ucByte);

int buf_get_buf_size(fixed_buf* pBuf);

/**
 * @brief 
 * 
 * @param pBuf 
 */
void buf_free_buf(fixed_buf* pBuf);

#define BUF_APPEND_CHAR(buf, c) \
  if(0 == buf_append_char(buf,c)) \
    return 0;

#endif