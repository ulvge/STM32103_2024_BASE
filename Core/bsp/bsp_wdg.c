/**
 ******************************************************************************
 * @file
 * @author
 * @version
 * @date
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "Types.h"
#include "debug_print.h"
#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct
{
    uint32_t cause;
    char *description;
} ResetCause;

IWDG_HandleTypeDef IwdgHandle;
uint32_t g_resetCause;

static uint32_t Record_resetCause(void);

static const ResetCause ResetCause_tb[] = {
    {0,                 "system reset reason: normal reset\r\n"},
    {RCC_FLAG_IWDGRST, "system reset reason: FWDG reset\r\n"},
    {RCC_FLAG_WWDGRST, "system reset reason: WWDGT reset\r\n"},
    {RCC_FLAG_PORRST, "system reset reason: power reset\r\n"},
    {RCC_FLAG_SFTRST, "system reset reason: soft reset\r\n"},
    {RCC_FLAG_PINRST, "system reset reason: external PIN \r\n"},
    {RCC_FLAG_LPWRRST, "system reset reason: low-power reset\r\n"},
};

void WatchDog_init(void)
{
    // 时间计算(大概):Tout= IWDG_PRESCALER_128  350  = 1s
    IwdgHandle.Instance = IWDG;
    IwdgHandle.Init.Prescaler = IWDG_PRESCALER_128;
    IwdgHandle.Init.Reload    = 350;

    if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
    {
        LOG_W("HAL_IWDG_Init  error\r\n");
    }

    g_resetCause = Record_resetCause();
    /* 复位标志清除，防止下一次复位时状态混乱 */
    __HAL_RCC_CLEAR_RESET_FLAGS();
}
void WatchDog_feed(void)
{
    HAL_IWDG_Refresh(&IwdgHandle);
}

static uint32_t Record_resetCause(void)
{
    /* check if the system has resumed from a specific reset */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        return RCC_FLAG_IWDGRST;  // 独立看门狗复位
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        return RCC_FLAG_WWDGRST;  // 窗口看门狗复位
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        return RCC_FLAG_PORRST;  // 上电复位
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        return RCC_FLAG_SFTRST;  // 软件复位
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        return RCC_FLAG_PINRST;  // 外部引脚复位
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        return RCC_FLAG_LPWRRST;  // 低功耗复位
    } else {
        return 0;  // 没有找到匹配的复位原因
    }
}

void ResetCause_Printf(void)
{
    for (uint32_t i = 0; i < ARRARY_SIZE(ResetCause_tb); i++)
    {
        if (ResetCause_tb[i].cause == g_resetCause) {
            LOG_RAW("\r\n");
            LOG_W("%s\r\n", ResetCause_tb[i].description);
            break;
        }
    }
}

uint32_t ResetCause_Get(void)
{
    return g_resetCause;
}


