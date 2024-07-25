#include <string.h>  
#include <stdlib.h>
#include <main.h>
#include "FreeRTOS.h"
#include "debug_print.h"
#include "print_monitor.h"
#include "bsp_uartcomm.h"
#include "bsp_usart1.h"
#include "bsp_i2c.h"
#include "stm32f1xx_hal.h"


extern I2C_HandleTypeDef hi2c1;

typedef struct
{
    USART_TypeDef * usart_periph;
} MsgUartResend_T;

static uint8_t g_I2cSendBuf[200];
static uint8_t g_I2cRecvBuf;
// 这个函数用法，需要接收数据的时候才用。收到数据中断之后，需要重复打开
// 使能之后，一直处于接收状态，无法发送
static void I2CFroward_StartRecv(void)
{
    //HAL_I2C_Slave_Receive_IT(&hi2c1, &g_I2cRecvBuf, sizeof(g_I2cRecvBuf));

    I2C_HandleTypeDef *hi2c = &hi2c1;
    uint8_t *pData = &g_I2cRecvBuf;
    uint16_t Size = sizeof(g_I2cRecvBuf);


    /* Check if the I2C is already enabled */
    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      __HAL_I2C_ENABLE(hi2c);
    }

    /* Disable Pos */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

    //hi2c->State     = HAL_I2C_STATE_BUSY_RX;
    //hi2c->Mode      = HAL_I2C_MODE_SLAVE;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr    = pData;
    hi2c->XferCount   = Size;
    hi2c->XferSize    = hi2c->XferCount;
    hi2c->XferOptions = 0xFFFF0000U;    // I2C_NO_OPTION_FRAME

    /* Enable Address Acknowledge */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

    /* Note : The I2C interrupts must be enabled after unlocking current process
              to avoid the risk of I2C interrupt handle execution before current
              process unlock */

    /* Enable EVT, BUF and ERR interrupt */
    __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);

    return;
}

// 把收到的所有数据，按固定的长度进行拆分，然后发送出去。
// 每次发送的数据长度为 32 字节，如果最后不足 32 字节，则发送剩余的字节。
//如果接收到的是文件，则不进行末尾字符判断
uint32_t i2cSend_split(void)
{
    #define PACKAGE_MAX_SIZE 15
    #define PRINT_MAX_SIZE PACKAGE_MAX_SIZE
    FIFO *fifo= &g_UARTPara.fifo.rfifo;
    int32_t packageSize; //把所有数据 按PACKAGE_MAX_SIZE 大小进行拆分，或者拆分到末尾
    uint32_t sendSize; // 按packageSize的大小，进行FIFO  Tail 判断，如果在Tail，继续二次拆分
	uint32_t printHeadSize;
    uint32_t sendTotal = 0;

    while( fifo->occupy > 0){    // 总长
        packageSize = fifo->occupy > PACKAGE_MAX_SIZE ? PACKAGE_MAX_SIZE : fifo->occupy;
        bool isTail = fifo->rp + packageSize >= fifo->limit;
        if (isTail) {
            sendSize = fifo->limit - fifo->rp;
            //printf("\r\n is Tail\r\n");
        }else{
            sendSize = packageSize;
        }

        //i2cs0_write_bytes(DEV_ADDR, fifo->rp, sendSize);
        // debug start
        if (0){
            printf("\r\n i2c send : from fifo->rp = %#x, size = %d\r\n", fifo->rp, sendSize);

            printf("\t hex : ");
            printHeadSize = sendSize >= PRINT_MAX_SIZE ? PRINT_MAX_SIZE : sendSize;
            for (size_t i = 0; i < printHeadSize; i++)
            {
                printf("%x ", fifo->rp[i]);
            }
            printf("\r\n\t char : ");
            for (size_t i = 0; i < printHeadSize; i++)
            {
                printf("%c", fifo->rp[i]);
            }
        }
        // debug end
        uint32_t x=API_EnterCirtical();
        fifo->occupy -= sendSize;
        if (isTail) {
            fifo->rp = fifo->array;
        }else{
            fifo->rp += sendSize;
        }
        API_ExitCirtical(x);
        sendTotal += sendSize;
    }
    return sendTotal;
}
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
#define MAX_RECV_COUNT 3
void Task_I2cForWard(void *param)
{
    uint32_t cpyCount;
    FIFO *rfifo= &g_UARTPara.fifo.rfifo;

    I2CFroward_StartRecv();
    while(1)
    {
        if (FIFO_Empty(rfifo)) {
            vTaskDelay(5);
            continue;
        }
        // 3 Uart Rx
        // copy from uart fifo to i2c send buf
        if (!I2CForward_copyFromUartFifo2Buff(g_I2cSendBuf, sizeof(g_I2cSendBuf), &cpyCount)){
            //printf("I2CForward_copyFromUartFifo2Buff failed\r\n");
            continue;
        }
        for (uint32_t i = 0; i < MAX_RECV_COUNT; i++)
        {
            // 4 I2C send
//            if (i2c_write_bytes(I2C_DEV_IPMB_ADDR, g_I2cSendBuf, cpyCount)){
//                vTaskDelay(10);
//                continue;
//            }

            // // I2C wait send finished
            // while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
            // {
            //     vTaskDelay(1);
            // }
            // //I2CFroward_StartRecv(); //switch recv mode
            // // I2C recv
            // if (HAL_I2C_Slave_Receive(&hi2c1, &g_I2cRecvBuf, sizeof(g_I2cRecvBuf), 1 * 1000) != HAL_OK){
            //     //printf("HAL_I2C_Slave_Receive failed\r\n");
            //     vTaskDelay(10);
            //     continue;
            // }
            // // uart ack send
            // printf("%d", g_I2cRecvBuf);
            break;
        }
    }
}
/*
    PC                  中转MCU                     目标MCU
            uart                        I2C

1                                <---枚举握手包
2      <---枚举握手包 
3      ack/data--->        
4                                ack/data--->

1 I2c Recv
2 Uart Tx
3 Uart Rx
4 I2c send
*/
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c1)
    {
        // 1 I2c Recv
        g_I2cRecvBuf = (uint8_t)hi2c->Instance->DR;
        // 2 Uart Tx
        printf("%d", g_I2cRecvBuf);
        //I2CFroward_StartRecv();
    }
}



