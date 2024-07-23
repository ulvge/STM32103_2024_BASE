#include <string.h>
#include "main.h"
#include "bsp_uartcomm.h"
#include "bsp_usart1.h"
#include "task.h"
#include "freertos.h"
#include "semphr.h"
#include "print_monitor.h"

#define UART_NUM_TOTAL 1

static UART_PARA_STRUCT *g_pUARTSHandler[UART_NUM_TOTAL] = {NULL};	
bool g_isPrintUseFifo = true;

//use FIFO
int fputc(int ch, FILE *f)
{
    if (g_isPrintUseFifo) {
        return UART_sendByte(DEBUG_UART_PERIPH, ch);
    } else {
        return UART_sendDataBlock(DEBUG_UART_PERIPH, (uint8_t *)&ch, 1);
    }
}
bool com_registHandler(UART_PARA_STRUCT *uartPara)
{
    uint32_t i;
    for (i = 0; i < UART_NUM_TOTAL; i++)
    {
        if (g_pUARTSHandler[i] == NULL){
            break;
        }
        if (g_pUARTSHandler[i]->periph == uartPara->periph) {
            return true; //alread exist
        }
    }
    if (i < UART_NUM_TOTAL) {
        g_pUARTSHandler[i] = uartPara;
        return true; //success
    } else{
        return false;
    }
}
UART_PARA_STRUCT *com_getHandler(USART_TypeDef *usart_periph)
{
    for (uint32_t i = 0; i < UART_NUM_TOTAL; i++)
    {
        if (g_pUARTSHandler[i]->periph == usart_periph) {
            return g_pUARTSHandler[i]; //alread exist
        }
        if (g_pUARTSHandler[i] == NULL){
            break;
        }
    }
    return NULL; //alread exist
}

bool UART_getByte(USART_TypeDef *usart_periph, uint8_t *p_buffer)
{
    UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
    if (uartPara == NULL) {
        return false;
    }
    return FIFO_Read(&uartPara->fifo.rfifo, p_buffer);
}

bool UART_getData(USART_TypeDef *usart_periph, uint8_t *p_buffer, uint32_t buffSize, INT16U *retLen)
{
    UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
    if (uartPara == NULL) {
        return false;
    }
    return FIFO_ReadN(&uartPara->fifo.rfifo, p_buffer, buffSize, (INT16U *)retLen);
}


__inline bool UART_sendByte(USART_TypeDef *usart_periph, uint8_t dat)
{
    return UART_sendData(usart_periph, &dat, 1);
}

bool UART_sendData(USART_TypeDef *usart_periph, uint8_t *str, uint16_t len)
{
    UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
    if (uartPara == NULL) {
        return false;
    }
    if (uartPara->dmaUsed == false) {
        if (FIFO_Writes(&uartPara->fifo.sfifo, str, len) == FALSE){
			UART_sendDataBlock(usart_periph, str, len);
            return false;
        }

        if(uartPara->fifo.status != UART_SENDING) {
            UART_sendContinueIT(usart_periph, &uartPara->fifo);
        }
        return true;
    }else{ //use DMA
        if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
            UART_sendDataBlock(usart_periph, str, len);
        }else{
            if (FIFO_Writes(&uartPara->fifo.sfifo, str, len) == FALSE){ // write failed
                HAL_UART_Transmit_DMA(uartPara->uartHandle, (uint8_t*)str, len); // transmission directly
                return false;
            }else{
                PrintMonitor_PostdMsg(usart_periph, false); // transmission later
            }
        }
        return true;
    }
}
/// @brief NO fifo,can be called in HardFault_Handler
/// @param usart_periph 
/// @param str 
/// @param len 
bool UART_sendDataBlock(USART_TypeDef *usart_periph, const uint8_t *str, uint16_t len)
{
    UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
    if (uartPara == NULL) {
        return false;
    }
    //usart_periph->DR = (uint8_t)(*pdata8bits & 0xFFU);
    HAL_UART_Transmit(uartPara->uartHandle, (uint8_t*)str, len, len);
    return true;
}

/// @brief read one byte from fifo,and start transmit
/// @param usart_periph 
/// @param fifoUart 
/// @return 
INT8U UART_sendContinueIT(USART_TypeDef * usart_periph, FIFO_Buf_STRUCT *fifoUart)
{
    INT8U data;
    if (FIFO_Empty(&(fifoUart->sfifo)))
    {
        fifoUart->status &= ~UART_SENDING;
        return false;
    }
    else
    {
        if (fifoUart->status != UART_SENDING)
        {
            fifoUart->status |= UART_SENDING;
        }
        if (FIFO_Read(&(fifoUart->sfifo), &data) == true)
        { // sending data
			UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
			if (uartPara == NULL) {
				return false;
			}
            HAL_UART_Transmit_IT(uartPara->uartHandle, &data, 1);
        }
        return true;
    }
}

void UART_sendContinueDMA(USART_TypeDef * usart_periph)
{
    INT16U sendSize;

    UART_PARA_STRUCT *uartPara = com_getHandler(usart_periph);
    if (uartPara == NULL) {
        return;
    }
    FIFO *fifo= &uartPara->fifo.sfifo;
    if (fifo->occupy == 0) {
		return;
	}
    // 1、 array      xxxxx   limit
    // 2、 array xx       xxx limit
    bool isNotTail = fifo->rp + fifo->occupy < fifo->limit;
    if (isNotTail) {
        sendSize = fifo->occupy;
    }else{
        sendSize = fifo->limit - fifo->rp;
    }

    /* wait DMA channel transfer complete */
    if(HAL_UART_Transmit_DMA(uartPara->uartHandle, fifo->rp, sendSize)!= true)
    {
        PrintMonitor_PostdMsg(usart_periph, true); // Send it later
    }else{
        uint32_t x=API_EnterCirtical();
        fifo->occupy -= sendSize;
        if (isNotTail) {
            fifo->rp += sendSize;
        }else{
            fifo->rp = fifo->array;
        }
        API_ExitCirtical(x);
    }
}
void UART_init(void)
{
    UART1_init();
}
