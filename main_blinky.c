/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart.h"
#include "serial/slip.h"
#include "Util/log.h"

static void prvQueueReceiveTask( void * pvParameters );
static void prvQueueSendTask( void * pvParameters );
static void prvReadUart0Task(void *); 

#define mainQUEUE_RECEIVE_TASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY       ( tskIDLE_PRIORITY + 1 )
#define mainQUEUE_LENGTH                   ( 1 )
#define mainQUEUE_SEND_FREQUENCY_MS        ( 200 / portTICK_PERIOD_MS )
/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;
static fixed_buf* log_buf = NULL; 

void main_blinky( void )
{
    /* Create the queue. */
    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

    if( xQueue != NULL )
    {
        /* Start the two tasks as described in the comments at the top of this
         * file. */
        xTaskCreate( prvQueueReceiveTask,             /* The function that implements the task. */
                     "Rx",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                     configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
                     NULL,                            /* The parameter passed to the task - not used in this case. */
                     mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
                     NULL );                          /* The task handle is not required, so NULL is passed. */

        xTaskCreate( prvQueueSendTask,
                     "TX",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     mainQUEUE_SEND_TASK_PRIORITY,
                     NULL );

        xTaskCreate( prvReadUart0Task,
                      "Uart0",
                      configMINIMAL_STACK_SIZE,
                      NULL,
                      mainQUEUE_RECEIVE_TASK_PRIORITY,
                      NULL);

        log_buf = buf_init_buf(1024);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the Idle and/or
     * timer tasks to be created.  See the memory management section on the
     * FreeRTOS web site for more details on the FreeRTOS heap
     * https://www.FreeRTOS.org/a00111.html. */
    for( ; ; )
    {
    }
}

static void prvQueueSendTask( void * pvParameters )
{
    TickType_t xNextWakeTime;
    const uint32_t ulValueToSend = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

        /* Send to the queue - causing the queue receive task to unblock and
         * toggle the LED.  0 is used as the block time so the sending operation
         * will not block - it shouldn't need to block as the queue should always
         * be empty at this point in the code. */
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}

volatile uint32_t ulRxEvents = 0;
static void prvQueueReceiveTask( void * pvParameters )
{
    uint32_t ulReceivedValue;
    const uint32_t ulExpectedValue = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    for( ; ; )
    {
        /* Wait until something arrives in the queue - this task will block
         * indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
         * FreeRTOSConfig.h. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        /*  To get here something must have been received from the queue, but
         * is it the expected value?  If it is, toggle the LED. */
        if( ulReceivedValue == ulExpectedValue )
        {
            printf( "%s\n", "blinking" );
            vTaskDelay( 1000 );
            ulReceivedValue = 0U;
            ulRxEvents++;
        }
    }
}

void do_print( const char * fmt,
                    ... )
{
    va_list vargs;

    va_start( vargs, fmt );
    vprintf( fmt, vargs );
    va_end( vargs );

    int len = strlen(fmt);
    Uart0Write(fmt, len);
}


void prvSlipPacketRcv(uint8_t* pData, int nDataLen) 
{
    slip_encode_slip_data(pData, nDataLen, log_buf);
    Uart0Write(log_buf->pData, log_buf->nDataLen);
    // printf("write slip data len %d\n", log_buf->nDataLen);
    // fixed_buf* _log = buf_init_buf(1024);
    // data_to_hex(log_buf->pData, log_buf->nDataLen, _log);
    // data_to_hex(pData, nDataLen, _log);
    // buf_append_char(_log, '\n');
    // Uart0Write(_log->pData, _log->nDataLen);
    // do_print("encode data : %d\n", log_buf->nDataLen);
    log_buf->nDataLen = 0;
    // buf_free_buf(_log);
}

#define READ_BUF_SIZE 1024
static void prvReadUart0Task(void *) 
{
    uint8_t readBuf[READ_BUF_SIZE];
    int readLen = 0;
    slip_handle_t slip = slip_create_rcv_handle(1024, prvSlipPacketRcv);

    for(;;) 
    {
        readLen = Uart0Read(readBuf, READ_BUF_SIZE);
        if (readLen == 0)
        {
            vTaskDelay(10);
            continue;
        }

        slip_rcv_serial_data(slip, readBuf, readLen);
        // readBuf[readLen] = '\0';
        // printf("%s", readBuf);
    }
}
/*-----------------------------------------------------------*/
