#ifndef __uart__H_
#define __uart__H_

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "Util/lwrb/lwrb.h"

struct __uart
{
    volatile uint32_t data;
    volatile uint32_t state;
    volatile uint32_t ctrl;
    volatile uint32_t int_status;
    volatile uint32_t bauddiv;
};

typedef struct __uart Uart_t;

struct __uart_config 
{
    uint32_t Base;
    SemaphoreHandle_t write_mutex;
    SemaphoreHandle_t read_mutex;
    lwrb_t      uart_rb;
};

typedef struct __uart_config UartConfig_t;
typedef UartConfig_t* UartHandle_t;
/* ARM UART enumeration types */
enum arm_uart_error_t {
    ARM_UART_ERR_NONE = 0,      /*!< No error */
    ARM_UART_ERR_INVALID_ARG,   /*!< Error invalid input argument */
    ARM_UART_ERR_INVALID_BAUD,  /*!< Invalid baudrate */
    ARM_UART_ERR_NOT_INIT,      /*!< Error UART not initialized */
    ARM_UART_ERR_NOT_READY,     /*!< Error UART not ready */
};

enum arm_uart_irq_t {
    ARM_UART_IRQ_RX,          /*!< RX interrupt source */
    ARM_UART_IRQ_TX,          /*!< TX interrupt source */
    ARM_UART_IRQ_COMBINED,    /*!< RX-TX combined interrupt source */
    ARM_UART_IRQ_NONE = 0xFF  /*!< RX-TX combined interrupt source */
};

enum arm_uart_error_t UartGetC(UartHandle_t handle, uint8_t* byte);

enum arm_uart_irq_t UartGetInterruptStatus(UartHandle_t handle);

void UartClearInterrupt(UartHandle_t handle, enum arm_uart_irq_t irq);

void UartInit();

#define UART0()           ( ( Uart_t * ) ( 0x40004000 ) )
#define UART0_BASE           0x40004000 

#define UART_STATE_TXFULL    ( 1 << 0 )
#define UART_CTRL_TX_EN      ( 1 << 0 )
#define UART_CTRL_RX_EN      ( 1 << 1 )
#define UART_INITIALIZED     ( 1 << 0 )

/*!
 * @brief 
 */
void Uart0RcvIsrHandler();
void Uart0SndIsrHandler();

/*!
 * @brief 
 * @param handle 
 * @param buf 
 * @param bufLen 
 * @return 
 */
int UartRead(UartHandle_t handle, uint8_t* buf, int bufLen);

int UartWrite(UartHandle_t handle, uint8_t* buf, int bufLen);

/*!
 * @brief 
 * @param buf 
 * @param bufLen 
 * @return 
 */
int Uart0Read(uint8_t* buf, int bufLen);

/**
 * @brief 
 * 
 * @param pBuf 
 * @param nBufLen 
 * @return int 
 */
int Uart0Write(uint8_t* pBuf, int nBufLen);

#ifdef __cplusplus
    }
#endif

#endif