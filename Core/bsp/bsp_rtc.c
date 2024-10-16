#include "api_utc.h"
#include "debug_print.h"
#include "stm32f1xx_hal.h"
#include <main.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "bsp_wdg.h"
#include "initcall.h"

static RTC_HandleTypeDef g_hrtc;
/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
void RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef DateToUpdate = {0};

    /** Initialize RTC Only
     */
    g_hrtc.Instance = RTC;
    g_hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    g_hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
    if (HAL_RTC_Init(&g_hrtc) != HAL_OK) {
        Error_Handler();
    }
    /** Initialize RTC and set the Time and Date
     */
    uint32_t cause = ResetCause_Get();
    if (cause == RCC_FLAG_PORRST) {
        sTime.Hours = 0x11;
        sTime.Minutes = 0x34;
        sTime.Seconds = 0x20;

        DateToUpdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
        DateToUpdate.Month = RTC_MONTH_OCTOBER;
        DateToUpdate.Date = 0x16;
        DateToUpdate.Year = 0x24; // 2024

        HAL_RTC_WaitForSynchro(&g_hrtc); // 等待同步
        __HAL_RTC_WRITEPROTECTION_DISABLE(&g_hrtc); // 禁止写入保护
        HAL_RTC_SetDate(&g_hrtc, &DateToUpdate, RTC_FORMAT_BCD); // 设置日期
        HAL_RTC_SetTime(&g_hrtc, &sTime, RTC_FORMAT_BCD); // 设置时间
        __HAL_RTC_WRITEPROTECTION_ENABLE(&g_hrtc); // 重新启用写入保护
    }
}
/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param g_hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef *g_hrtc)
{
    if (g_hrtc->Instance == RTC) {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */
        HAL_PWR_EnableBkUpAccess();
        /* Enable BKP CLK enable for backup registers */
        __HAL_RCC_BKP_CLK_ENABLE();
        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
        /* RTC interrupt Init */
        // HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(RTC_IRQn);
        /* USER CODE BEGIN RTC_MspInit 1 */

        /* USER CODE END RTC_MspInit 1 */
    }
}
void RTC_GetDate(RTC_TimeTypeDef *time, uint32_t Format)
{
    HAL_RTC_GetTime(&g_hrtc, time, Format);
}
static void DateUsage(void)
{
    LOG_RAW("date ----------get current date time\r\n");
    LOG_RAW("date YYYY-MM-DD HH:MM:SS----------set date time\r\n");
}

/*!
    \brief      get set the date time
    \param[in]  none
    \param[out] none
    \retval     none
*/
int RTC_DateTime(int argc, char *argv[])
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    if (argc == 1) {
        HAL_RTC_GetTime(&g_hrtc, &time, RTC_FORMAT_BCD);
        HAL_RTC_GetDate(&g_hrtc, &date, RTC_FORMAT_BCD);
        
        LOG_RAW("Current time: 20%0.2x-%0.2x-%0.2x %0.2x:%0.2x:%0.2x\n\r",
                date.Year, date.Month, date.Date,
                time.Hours, time.Minutes, time.Seconds);
    } else if (argc == 3) {
        if (argv[1] == NULL || argv[2] == NULL) {
            return 0;
        }
        int year, month, day, hour, minute, second;
        sscanf(argv[1], "%4d-%2d-%2d", 
           &year, &month, &day);
        sscanf(argv[2], "%2d:%2d:%2d", 
            &hour, &minute, &second);

        date.Year = year % 100;
        date.Month = month;
        date.Date = day;
        time.Hours = hour;
        time.Minutes = minute;
        time.Seconds = second;
        
        HAL_RTC_WaitForSynchro(&g_hrtc); // 等待同步
        __HAL_RTC_WRITEPROTECTION_DISABLE(&g_hrtc); // 禁止写入保护
        HAL_RTC_SetDate(&g_hrtc, &date, RTC_FORMAT_BIN); // 设置日期
        HAL_RTC_SetTime(&g_hrtc, &time, RTC_FORMAT_BIN); // 设置时间
        __HAL_RTC_WRITEPROTECTION_ENABLE(&g_hrtc); // 重新启用写入保护
    } else {
        DateUsage();
    }
    return 0;
}

CoreInitCall(RTC_Init);
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, date, RTC_DateTime, get / set date & time);
