#ifndef __PRINT_MONITOR_H
#define	__PRINT_MONITOR_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "semphr.h"

#define UART_MONITOR_DELAY 10

 void PrintMonitorInit(void);
 void PrintMonitor_PostdMsg(USART_TypeDef * usart_periph, bool isReSend);
 void Task_uartMonitor(void *param);

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_MONITOR_H */

