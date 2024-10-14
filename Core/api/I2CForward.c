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


void Task_I2cForWard(void *param)
{
    while(1)
    {
        vTaskDelay(5);
    }
}


