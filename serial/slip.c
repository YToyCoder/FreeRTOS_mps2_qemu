#include "slip.h"

#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "portable.h"

#define SLIP_END 0xC0
#define SLIP_ESC 0xdb
#define SLIP_END_ESC 0xdc
#define SLIP_ESC_ESC 0xdd

typedef struct internel_slip
{
  SLIP_FIELD();
  int   nDataMaxLen;
} internel_slip_t;


/**
 * @brief 
 * 
 * @return int 
 */
int slip_data_decode(uint8_t*, int);
void slip_rcv_packet_too_large_mark(slip_handle_t);
int slip_append_data(slip_handle_t, uint8_t*, int);
void slip_handle_packet_cb(slip_handle_t handle, uint8_t* pData, int nDataLen);

int slip_get_cache_len(slip_handle_t hd1)
{
  if (NULL == hd1)
    return 0;

  internel_slip_t* hd = (internel_slip_t*) hd1;
  return hd->nDataMaxLen;
}

void slip_decode_cache_data_packet(slip_handle_t handle) 
{
  if (handle == NULL || handle->nDataLen == 0) 
    return;
  
  int decode_len = slip_data_decode(handle->uData, handle->nDataLen);
  slip_handle_packet_cb(handle, handle->uData, decode_len);
  handle->nDataLen = 0;
}

int slip_find_ESC(uint8_t* pData, int nDataLen) 
{
  for (int i = 0; i < nDataLen; i++) {
    if (pData[i] == SLIP_ESC) {
      return i;
    }
  }

  return -1;
}

void slip_handle_packet_cb(slip_handle_t handle, uint8_t* pData, int nDataLen)
{
  if (handle == NULL || handle->packet_rcv_cb == NULL) 
    return;
  
  handle->packet_rcv_cb(pData, nDataLen);
}

uint8_t slip_ESC_decode(uint8_t* pData, int nPos, int nDataLen) {
  switch(pData[nPos + 1]){
    case SLIP_END_ESC:
      return 0xc0;
    case SLIP_ESC_ESC:
      return 0xdb;
  }

  // should not reach here!
  return pData[nPos];
} 

int slip_data_decode(uint8_t* pData, int nDataLen)
{
  int cur_head = 0;
  int decode_pos = slip_find_ESC(pData, nDataLen);
  int decode_ind = 0;

  if (-1 == decode_pos) {
    return nDataLen;
  }

  for (; decode_pos < nDataLen; decode_ind++) {
    if (pData[decode_pos] == SLIP_ESC) {
      pData[decode_ind] = slip_ESC_decode(pData, decode_pos, nDataLen);
      decode_pos += 2;
    }else {
      pData[decode_ind] = pData[decode_pos++];
    }
  }

  return decode_ind;
}

int slip_append_data(slip_handle_t handle, uint8_t* pData, int nDataLen)
{
  if (handle == NULL)
    return 0;

  int remaining_len = slip_get_cache_len(handle) - handle->nDataLen;
  if (remaining_len < nDataLen)
    return 0;

  memcpy(handle->uData + handle->nDataLen, pData, nDataLen);
  handle->nDataLen += nDataLen;
  return handle->nDataLen;
}

slip_handle_t slip_create_rcv_handle(int nMaxBufSlip, slip_packet_cb cb)
{
  internel_slip_t* handle = pvPortMalloc(sizeof(internel_slip_t));

  if (handle == NULL)
    return NULL;

  handle->uData = pvPortMalloc(nMaxBufSlip);

  if (handle->uData == NULL) {
    vPortFree(handle);
    return NULL;
  }

  handle->nDataLen = 0;
  handle->nDataMaxLen = nMaxBufSlip;
  handle->packet_rcv_cb = cb;
  handle->semaphore = xSemaphoreCreateMutex();
  return handle;
}

int find_slip_end(uint8_t* pData, int nDataLen) 
{
  int ind = -1;
  for (int i = 0; i < nDataLen; i++) {
    if (pData[i] == SLIP_END) {
      ind = i;
      break;
    }
  }

  return ind;
}

int find_slipEND_head(uint8_t* pData, int nDataLen) 
{
  int first_END_ind = find_slip_end(pData, nDataLen);

  if (first_END_ind == -1) 
    return -1;

  if (first_END_ind + 1 != nDataLen && pData[first_END_ind + 1] == SLIP_END)
    return first_END_ind + 1;

  return first_END_ind;
}

void slip_rcv_packet_too_large_mark(slip_handle_t handle) {
  handle->uData[0] = SLIP_END;
  handle->nDataLen = 1;
}

static void rcv_slip_data(slip_handle_t handle, uint8_t* pData, int nDataLen)
{
  if (handle == NULL) 
    return;

  int remaining_size = slip_get_cache_len(handle) - handle->nDataLen;

  if (handle->nDataLen == 0) {
    int slip_end_idx = find_slip_end(pData, nDataLen);
    if (-1 == slip_end_idx) {
      if (0 == slip_append_data(handle, pData, nDataLen))
        slip_rcv_packet_too_large_mark(handle);
      return;
    }

    int slip_packet_len = slip_data_decode(pData, slip_end_idx);
    if (slip_packet_len != 0) 
      slip_handle_packet_cb(handle, pData, slip_packet_len);

    if (slip_end_idx + 1 < nDataLen) 
      rcv_slip_data(handle, pData + slip_end_idx + 1, nDataLen - 1 - slip_end_idx);

    return;
  }

  int slip_end_ind = find_slip_end(pData, nDataLen);

  if (slip_end_ind == -1) {
    if (nDataLen > remaining_size) {
      // packet is too large , mark it, when rcv packet just drop it!
      slip_rcv_packet_too_large_mark(handle);
      return;
    }

    slip_append_data(handle, pData, nDataLen);
    return;
  }

  slip_append_data(handle, pData, slip_end_ind);
  slip_decode_cache_data_packet(handle);

  if (nDataLen - slip_end_ind - 1 > 0) {
    rcv_slip_data(handle, pData + slip_end_ind + 1, nDataLen - slip_end_ind - 1);
  }
}

void slip_rcv_serial_data(slip_handle_t handle, uint8_t* pData, int nDataLen) 
{
  xSemaphoreTake(handle->semaphore, -1);
  rcv_slip_data(handle, pData, nDataLen);
  xSemaphoreGive(handle->semaphore);
}


int slip_encode_slip_data(uint8_t* pData, int nDataLen, fixed_buf* buf)
{
  if (pData == NULL || 0 == nDataLen || buf == NULL)
    return 0;

  if (buf_get_buf_size(buf) - buf->nDataLen < nDataLen + 2)
    return 0;

  buf_append_char(buf, SLIP_END);

  for (int i = 0; i < nDataLen; i++){
    switch(pData[i])
    {
      case SLIP_END:
        BUF_APPEND_CHAR(buf, SLIP_ESC);
        BUF_APPEND_CHAR(buf, SLIP_END_ESC);
        break;
      case SLIP_ESC:
        BUF_APPEND_CHAR(buf, SLIP_ESC);
        BUF_APPEND_CHAR(buf, SLIP_ESC_ESC);
        break;
      default:
        BUF_APPEND_CHAR(buf, pData[i]);
    }
  }

  BUF_APPEND_CHAR(buf, SLIP_END);

  return 1;
}