#include <string.h>  
#include <stdlib.h>
#include "FreeRTOS.h"
#include <main.h>
#include "debug_print.h"
#include "print_monitor.h"
#include "bsp_uartcomm.h"

static xQueueHandle g_Queue_uartResend = NULL;
typedef struct
{
    USART_TypeDef * usart_periph;
} MsgUartResend_T;

void PrintMonitorInit(void)
{
    g_Queue_uartResend = xQueueCreate(2, sizeof(MsgUartResend_T));
}
void Task_uartMonitor(void *param)
{
    MsgUartResend_T msg;
    while(1)
    {
        if (xQueueReceive(g_Queue_uartResend, &msg, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }
        vTaskDelay(UART_MONITOR_DELAY);
        UART_sendContinueDMA(msg.usart_periph);
    }
}

#define UART_RESEND_MAX_COUNT 2
void PrintMonitor_PostdMsg(USART_TypeDef * usart_periph, bool isReSend)
{
    static uint32_t errCount = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    MsgUartResend_T msg;
    BaseType_t err;

    UBaseType_t msgCount;
    if (vPortGetIPSR()) {
        msgCount = uxQueueMessagesWaitingFromISR(g_Queue_uartResend);
    }else{
        msgCount = uxQueueMessagesWaiting(g_Queue_uartResend);
    }
    if (msgCount != 0)
    {
        return;
    }
    if (isReSend){
        if (++errCount > UART_RESEND_MAX_COUNT){
            return;
        }else{
            //LOG_E("When uart sends data, repeated transmission occurs\r\n");
        }
    }else{
        errCount = 0;
    }
    msg.usart_periph = usart_periph;
    if (vPortGetIPSR()) {
        err = xQueueSendFromISR(g_Queue_uartResend, (char*)&msg, &xHigherPriorityTaskWoken);
    }else {
        err = xQueueSend(g_Queue_uartResend, (char*)&msg, 0);
    }
    if (err == pdFAIL)
    {
        //LOG_E("PrintMonitor PostdMsg failed\r\n");
		errCount = errCount;
    }
}
