#ifndef __slip__H_
#define __slip__H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#include "Util/buffer/buffer.h"

/**
 * @brief a function pointer type that use to process decoded slip data packet
 */
typedef void(*slip_packet_cb)(uint8_t*, int);

#define SLIP_FIELD() \
  slip_packet_cb    packet_rcv_cb; \
  SemaphoreHandle_t semaphore;  \
  int               nDataLen;   \
  uint8_t*          uData;      \

/**
 * @brief slip read data handle
 */
struct __slip {
  SLIP_FIELD();
};

typedef struct __slip slip_t;
typedef slip_t* slip_handle_t;

/**
 * @brief create a slip protocol receive serial data handle, 
 *        which cache received data, decode data and packet to call back function. 
 *        That means the handle need a slip packet process call back funtion to do real slip
 *        packet processing.
 * @param nMaxBufSize max size of cache data
 * @param cb          call back function that process slip packet
 * @return slip_handle_t 
 */
slip_handle_t slip_create_rcv_handle(int nMaxBufSize, slip_packet_cb cb);

/**
 * @brief process received serial data that packet slip data
 * 
 * @param handle slip receive serial data handle
 * @param pData receive data
 * @param nDataLen received data length
 */
void slip_rcv_serial_data(slip_handle_t handle, uint8_t* pData, int nDataLen);

int slip_encode_slip_data(uint8_t* pData, int nDataLen, fixed_buf* buf);

int slip_get_cache_len(slip_handle_t);

#endif