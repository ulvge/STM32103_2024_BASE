#include <string.h>  
#include <stdlib.h>
#include <main.h>
#include "FreeRTOS.h"
#include "debug_print.h"
#include "print_monitor.h"
#include "bsp_uartcomm.h"
#include "bsp_usart1.h"
#include "bsp_i2c.h"

#define EEPROM_DEV_ADDR			0xA0
typedef struct
{
    USART_TypeDef * usart_periph;
} MsgUartResend_T;

static uint8_t g_I2cSendBuf[200];
/// @brief 按收到的包，进行完整转发，不拆包。规则是：收到数据包，并空闲后，再转发，不空闲，一直收
/// @param  
/// @return 
static bool I2CForward_copyFromUartFifo2Buff(uint8_t *buf, uint32_t len, uint32_t *cpyCount)
{
    uint32_t x;
    FIFO *fifo= &g_UARTPara.fifo.rfifo;
    uint32_t occupyBak = fifo->occupy;
    uint32_t res;
    uint32_t sendSize = 0;
    while (1)
    {
        vTaskDelay(1);
        if (occupyBak != fifo->occupy){
            occupyBak = fifo->occupy;
        }else{
			break;
		}
    }

    bool isTail = fifo->rp + fifo->occupy >= fifo->limit;

    if (isTail) {
        sendSize = fifo->limit - fifo->rp;
        if (sendSize > len){
            return false;
        }
        memcpy(buf, fifo->rp, sendSize);

        x=API_EnterCirtical();
        fifo->occupy -= sendSize;
        fifo->rp = fifo->array; //reset head
        API_ExitCirtical(x);

        //printf("\r\n is Tail\r\n");
    }
    res = fifo->occupy;

    if (sendSize + res > len){
        return false;
    }
    memcpy(buf + sendSize, fifo->rp, res);

    x=API_EnterCirtical();
    fifo->occupy -= res;
    fifo->rp += res;
    API_ExitCirtical(x);

    *cpyCount = sendSize + res;
    return true;
}
void Task_I2cForWard(void *param)
{
    uint32_t cpyCount, recvCount = 0;
    FIFO *rfifo= &g_UARTPara.fifo.rfifo;
    while(1)
    {
        // if (xQueueReceive(g_Queue_uartResend, &msg, portMAX_DELAY) != pdTRUE)
        // {
        //     continue;
        // }

        if (FIFO_Empty(rfifo)) {
            vTaskDelay(10);
            continue;
        }
        // I2C copy
        if (I2CForward_copyFromUartFifo2Buff(g_I2cSendBuf, sizeof(g_I2cSendBuf), &cpyCount)){
            i2c_write_bytes(EEPROM_DEV_ADDR, g_I2cSendBuf, cpyCount);
            continue;
        }
        // I2C forward
        printf("rec:%d, i2c:%d\r\n", ++recvCount, cpyCount);
    }
}

