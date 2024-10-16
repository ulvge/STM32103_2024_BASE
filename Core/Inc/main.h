/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "freertos.h"
		

#define DEBUG_UART_PERIPH    USART1

#define    HARDWARE_VERSION     "1.0"
#define    SOFT_VERSION         "1.0"


// config IRQ HANDLER_PRIORITY
#define IRQHANDLER_PRIORITY_TICK        TICK_INT_PRIORITY
#define IRQHANDLER_PRIORITY_PENDSV      15

#define IRQHANDLER_PRIORITY_ADC         14
#define IRQHANDLER_PRIORITY_UART_DMA    14
#define IRQHANDLER_PRIORITY_UART        14

#define IRQHANDLER_PRIORITY_SPI_DMA     9
#define IRQHANDLER_PRIORITY_SPI         8

#define IRQHANDLER_PRIORITY_GPIO        5
// config IRQ HANDLER_PRIORITY end

extern BaseType_t xHigherPriorityTaskWoken_YES;
extern BaseType_t xHigherPriorityTaskWoken_NO;

extern int g_debugLevel;
/* Exported functions prototypes ---------------------------------------------*/
extern void Error_Handler(void);

extern uint32_t g_resetCause;
/* USER CODE BEGIN EFP */

#define SYS_CORE_CLOCK        72000000L  //并不真正起效，只是别处会调用这个值
#define SYS_CORE_CLOCK_X_MHZ        (SYS_CORE_CLOCK/1000000L)
#define SYS_CORE_CLOCK_X_KHZ        (SYS_CORE_CLOCK/1000L)
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
