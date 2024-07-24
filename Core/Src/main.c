/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "freertos.h"
#include "bsp_uartcomm.h"
#include "debug_print.h"
#include "bsp_gpio.h"
#include "shell.h"
#include "shell_port.h"
#include "api_utc.h"
#include "cm_backtrace.h"
#include "print_monitor.h"
#include "bsp_i2c.h"
#include "I2CForward.h"

/* Private function prototypes -----------------------------------------------*/
BaseType_t xHigherPriorityTaskWoken_YES = pdTRUE;
BaseType_t xHigherPriorityTaskWoken_NO = pdFALSE;
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;

void SystemClock_Config(void);

int g_debugLevel = DBG_LOG;
TaskHandle_t gp_xHandle_Task_uartMonitor = NULL;
TaskHandle_t gp_xHandle_Task_shell = NULL;
static __IO uint64_t g_utc_time_firmware_build = 0;
static const char *projectInfo =
    "\r\n"
    "********************************************\r\n"
    "************   BOARD   INFO      ************\r\n"
    "********************************************\r\n"
    "Build:    "__DATE__
    "  "__TIME__
    "\r\n"
    "Soft Version:  " SOFT_VERSION " \r\n"
    "Copyright: (c) HXZY\r\n"
    "********************************************\r\n"
    "\r\n";
    

static void DebugConfig(void)
{
    //DBGMCU_CR_DBG_IWDG_STOP();
    //DBGMCU_CR_DBG_WWDG_STOP();
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Uart Tx*/
    /* DMA1_Channel4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, IRQHANDLER_PRIORITY_UART_DMA, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    // I2C Tx
    /* DMA1_Channel6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, IRQHANDLER_PRIORITY_UART_DMA, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
}


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    g_utc_time_firmware_build = currentSecsSinceEpoch(__DATE__, __TIME__);

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init(); // set HAL_NVIC_SetPriorityGrouping
    // set AHB and APB buses clocks, 
    SystemClock_Config(); 

    /* Initialize all configured peripherals */
    GPIO_Init();
    MX_DMA_Init();

    i2c_int();
    UART_init();
	
    LOG_RAW("%s", projectInfo); 
    DebugConfig();
    LOG_RAW("init other peripherals over\r\n");
    /* CmBacktrace initialize */
    cm_backtrace_init("CmBacktrace", HARDWARE_VERSION, SOFT_VERSION);

    xTaskCreate(Task_I2cForWard, "forward", 128 * 4, NULL, 30, NULL );
    /* creation of uartMonitor */
    xTaskCreate(Task_uartMonitor, "uartMonitor", 128 * 4, NULL, 24, &gp_xHandle_Task_uartMonitor );
    /* creation of shell */
    //xTaskCreate(shellTask, "shell", 128 * 2, &shell, 16, &gp_xHandle_Task_shell );

    LOG_I("create all task finished and succeed\r\n");

    /* Start scheduler */
	vTaskStartScheduler();

    /* We should never get here as control is now taken by the scheduler */

    while (1) {
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
inline void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void PrintTaskStackHead(void)
{
    TaskHandle_t taskHandle[3] = {gp_xHandle_Task_uartMonitor, gp_xHandle_Task_shell};
    uint32_t taskStackWaterMark;

    LOG_D("\r\n");
    LOG_D("    OS,            MinHeapSize = %d\r\n", xPortGetMinimumEverFreeHeapSize());
    LOG_D("    task           StackHighWaterMark\r\n");
    for (int i = 0; i < 3; i++) {
        if (taskHandle[i] != NULL) {
            taskStackWaterMark = uxTaskGetStackHighWaterMark(taskHandle[i]) * 4;
            LOG_D("    %-15s %d\r\n", pcTaskGetTaskName(taskHandle[i]), taskStackWaterMark);
        }
    }
}
void vApplicationIdleHook( void )
{
    static bool isPrinted = false;
    uint32_t tickNow = HAL_GetTick();
    if (tickNow % 1000 == 0) {
        if (!isPrinted) {
            isPrinted = true;
            //PrintTaskStackHead();
        }
    }else{
        isPrinted = false;
    }
}
void vPortSetupTimerInterrupt( void )
{

}
void Reboot(void){
    HAL_NVIC_SystemReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, reboot, Reboot, reboot mcu);
