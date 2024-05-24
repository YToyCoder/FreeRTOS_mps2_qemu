#include "uart.h"

#define UART_BUF_SIZE 1024

uint8_t uart0_buf[UART_BUF_SIZE + 1];

UartConfig_t uart0_config;

enum arm_uart_error_t UartGetC(UartHandle_t handle, uint8_t* byte)
{
  Uart_t* uart = handle->Base;
  if((uart->state & UART_CTRL_RX_EN) == 0)
      return ARM_UART_ERR_NOT_READY;

  *byte =(uint8_t) uart->data;
  return ARM_UART_ERR_NONE;
}

enum arm_uart_error_t UartPutC(UartHandle_t hd, uint8_t byte)
{
  Uart_t* uart = hd->Base;
  if((uart->state & UART_CTRL_TX_EN))
      return ARM_UART_ERR_NOT_READY;

  uart->data = byte;
  return ARM_UART_ERR_NONE;
}

void Uart0RcvIsrHandler()
{
  // UartClearInterrupt(&uart0_config, UartGetInterruptStatus(&uart0_config) );
  if (UartGetInterruptStatus(&uart0_config) == ARM_UART_IRQ_RX) {
    UartClearInterrupt(&uart0_config, ARM_UART_IRQ_RX);
  }
}

void Uart0SndIsrHandler()
{
  if (UartGetInterruptStatus(&uart0_config) == ARM_UART_IRQ_TX) {
    UartClearInterrupt(&uart0_config, ARM_UART_IRQ_TX);
  }
}

void prvUart0RcvTask(void*)
{
  uint8_t rcvChar;
  for (;;) {

    if(UartGetC(&uart0_config, &rcvChar) != ARM_UART_ERR_NONE) 
    {
      vTaskDelay(1);
      continue;
    }
    
    lwrb_write(&uart0_config.uart_rb, &rcvChar, 1);
  }
}

void UartInit()
{
  uart0_config.Base = UART0_BASE;

  UART0()->bauddiv = 16;
  UART0()->ctrl = UART_CTRL_RX_EN | UART_CTRL_TX_EN;

  uart0_config.write_mutex = xSemaphoreCreateMutex();
  uart0_config.read_mutex = xSemaphoreCreateMutex();

  if(!lwrb_init(&uart0_config.uart_rb, uart0_buf, sizeof(uart0_buf))) {
    printf("uart0 rcv ring_buffer init error!\n");
  }

  xTaskCreate(prvUart0RcvTask, "tUart0Rcv", 2000, NULL, 2, NULL);
}

enum arm_uart_irq_t UartGetInterruptStatus(UartHandle_t handle)
{
  enum arm_uart_irq_t intr_status = ARM_UART_IRQ_NONE;
  Uart_t* uart = handle->Base;

  // if (handle->uart->state & UART_INITIALIZED) 
  if (uart->state & UART_INITIALIZED) 
  {
    // switch(handle->uart->int_status)
    switch(uart->int_status)
    {
      case UART_CTRL_TX_EN:
        intr_status = ARM_UART_IRQ_TX;
        break;
      case UART_CTRL_RX_EN:
        intr_status = ARM_UART_IRQ_RX;
        break;
      case UART_CTRL_RX_EN | UART_CTRL_TX_EN:
        intr_status = ARM_UART_IRQ_COMBINED;
        break;
    }
  }

  return intr_status;
}

void UartClearInterrupt(UartHandle_t handle, enum arm_uart_irq_t irq)
{
  Uart_t* uart = handle->Base;
  if (uart->state & UART_INITIALIZED) 
  {
    switch(irq)
    {
      case ARM_UART_IRQ_RX:
        uart->int_status = UART_CTRL_RX_EN;
        break;
      case ARM_UART_IRQ_TX:
        uart->int_status = UART_CTRL_TX_EN;
        break;
      case ARM_UART_IRQ_COMBINED:
        uart->int_status = UART_CTRL_TX_EN | UART_CTRL_RX_EN;
        break;
      case ARM_UART_IRQ_NONE:
        break;
    }
  }
}

int UartRead(UartHandle_t handle, uint8_t* buf, int bufLen)
{
  xSemaphoreTake(handle->read_mutex, -1);
  if (!lwrb_is_ready(&handle->uart_rb)) {
    xSemaphoreGive(handle->read_mutex);
    return 0;
  }

  int read_len = lwrb_read(&handle->uart_rb, buf, bufLen);

  xSemaphoreGive(handle->read_mutex);
  return read_len;
}

int Uart0Read(uint8_t* buf, int bufLen) 
{
  return UartRead(&uart0_config, buf, bufLen);
}

int UartWrite(UartHandle_t handle, uint8_t* pBuf, int nBufLen)
{
  if (NULL == handle || NULL == pBuf) 
    return 0;

  portDISABLE_INTERRUPTS();
  xSemaphoreTake(handle->write_mutex, -1);
  Uart_t* uart = handle->Base;
  for (int i = 0; i < nBufLen; i++)
    // UartPutC(handle, pBuf[i]);
    // uart->data = pBuf[i];
    while(UartPutC(handle, pBuf[i]) != ARM_UART_ERR_NONE);

  xSemaphoreGive(handle->write_mutex);
  portENABLE_INTERRUPTS();
  return 1;

}

int Uart0Write(uint8_t* pBuf, int nBufLen)
{
  return UartWrite(&uart0_config, pBuf, nBufLen);
}