/**
 * @file shell.c
 * @author Letter (NevermindZZT@gmail.com)
 * @version 3.0.0
 * @date 2019-12-30
 * 
 * @copyright (c) 2020 Letter
 * 
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "shell_ext.h"
#include "shell_port.h"
#include "bsp_gpio.h"
#include "debug_print.h"
#include "stdlib.h" 
#include "main.h" 
#include "task.h"
#include "print_monitor.h"
#include "pwm.h"
#include "bsp_rtc.h"
#include "initcall.h"
#include "alarm.h"
#include "bsp_key.h"

#define ALARM_MONITOR_DELAY 10
#define ALARM_TIME_X_MS (60*1000)   // 1分钟，如果没有人操作，则默认响铃1分钟

static void parse_arguments(int argc, char **argv);

static int AlarmSetHandler(int argc, char **argv, int index);
static int AlarmGetHandler(int argc, char **argv, int index);
static RTC_AlarmTypeDef g_alarmTime; // 闹钟时间
static RTC_HandleTypeDef *hrtcHandle; // RTC句柄

static SemaphoreHandle_t semaphoreNeedAlarm; // 闹钟触发信号量
SemaphoreHandle_t semaphoreStopAlarm; // 按键停止闹钟信号量

// shell start
static int (*const handlerList[])(int, char **, int) =  {
    AlarmSetHandler,
    AlarmGetHandler,
    NULL
};

static char *const arglist[] = {
    "-s",
    "-g",
    NULL
};

static void display_usage(void)
{
    LOG_RAW("alarm ----------get current alarm time\r\n");
    LOG_RAW("alarm HH:MM:SS----------set alarm time\r\n");
    return;
}

static void parse_arguments(int argc, char **argv)
{
    int i, j;

    if (argc <= 1)
    {
        display_usage();
        return;
    }

    for (i = 1; i < argc; i++)
    {
        j = 0;
        while (arglist[j] != NULL)
        {
            if (strcmp(argv[i], arglist[j]) == 0)
            {
                int retval;

                /* Match!  Handle this argument (and skip the specified
                   number of arguments that were just handled) */
                retval = handlerList[j](argc, argv, i);
                if (retval >= 0)
                    i += retval;
                else
                {
                    LOG_E("Cannot handle argument: %s\r\n", arglist[j]);
                }
            }
            j++;
        }
    }
}
static int AlarmGetHandler(int argc, char **argv, int index)
{
    LOG_RAW("Alarm time:  %02d:%02d:%02d\r\n", g_alarmTime.AlarmTime.Hours, g_alarmTime.AlarmTime.Minutes, g_alarmTime.AlarmTime.Seconds);
    return 0;
}
static int AlarmSetHandler(int argc, char **argv, int index)
{
    int hours, minutes, seconds;
    HAL_StatusTypeDef status;
    if (argc != 2){
        display_usage();
        return 0;
    }
    
    const char *end = argv[1];
    hours = strtol(end, (char**)&end, 10);  // 解析小时
    if (*end != ':' && *end != '.' && *end != '-') {
        LOG_RAW("Invalid time format\r\n");
        return 0;
    }

    end++;  // 跳过分隔符（.、:、-等）
    minutes = strtol(end, (char**)&end, 10); // 解析分钟
    if (*end != ':' && *end != '.' && *end != '-') {
        LOG_RAW("Invalid time format\r\n");
        return 0;
    }
    
    end++;
    seconds = strtol(end, (char**)&end, 10); // 解析秒
    if (*end != '\0') {
        LOG_RAW("Unexpected characters after seconds\r\n");
        return 0;
    }

    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59) {
        LOG_RAW("Invalid time format, please use HH:MM:SS\r\n");
        return 0;
    }

    g_alarmTime.AlarmTime.Hours = hours;     // 赋值小时
    g_alarmTime.AlarmTime.Minutes = minutes; // 赋值分钟
    g_alarmTime.AlarmTime.Seconds = seconds; // 赋值秒

    status = HAL_RTC_SetAlarm_IT(hrtcHandle, &g_alarmTime, RTC_FORMAT_BIN);
    if (status != HAL_OK) {
        LOG_RAW("Alarm time set failed, status:  %d\r\n", status);
    }else{
        LOG_RAW("Alarm time set success:  %02d:%02d:%02d\r\n", hours, minutes, seconds);
    }
    return 0;
}

static int Alarm_shell(int argc, char *argv[])
{
    parse_arguments(argc, argv);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, alarm, Alarm_shell, get/set alarm time);

// shell end
void RTC_enableIT(RTC_HandleTypeDef *hrtc)
{
    HAL_StatusTypeDef status;
    
    hrtcHandle = hrtc;
    g_alarmTime.AlarmTime.Hours = 12;    // 闹钟触发的小时
    g_alarmTime.AlarmTime.Minutes = 0;   // 分钟
    g_alarmTime.AlarmTime.Seconds = 0;   // 秒

    /* 1. 先配置中断优先级（避免配置过程中触发中断） */
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, TICK_INT_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);

    /* 2. 禁用写入保护（严格包裹所有RTC寄存器操作） */
    __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);

    /* 3. 检查并配置EXTI线（确保无冲突） */
    if (__HAL_RTC_ALARM_EXTI_GET_FLAG() != SET) {
        __HAL_RTC_ALARM_EXTI_ENABLE_IT();
        __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();
        __HAL_RTC_ALARM_EXTI_CLEAR_FLAG(); // 清除可能的残留标志
    }

    /* 4. 使能RTC Alarm中断 */
    __HAL_RTC_ALARM_ENABLE_IT(hrtc, RTC_IT_ALRA);

    /* 5. 设置闹钟时间 */
    status = HAL_RTC_SetAlarm_IT(hrtc, &g_alarmTime, RTC_FORMAT_BIN);
    if (status != HAL_OK) {
        // 错误处理（如打印日志或复位RTC）
        Error_Handler();
    }

    /* 6. 重新启用写入保护 */
    __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
}
void Alarm_Task(void *param)
{
    int alarmCount; 
    RTC_TimeTypeDef systime;

    semaphoreNeedAlarm = xSemaphoreCreateBinary();
    semaphoreStopAlarm = xSemaphoreCreateBinary();
    while(1)
    {
        if(xSemaphoreTake(semaphoreNeedAlarm, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        // recheck alarm time
        RTC_GetDate(&systime, RTC_FORMAT_BIN);
        if (g_alarmTime.AlarmTime.Hours != systime.Hours || g_alarmTime.AlarmTime.Minutes != systime.Minutes) {
            LOG_I("Alarm time not match, AlarmTime=%02d:%02d:%02d, current=%02d:%02d:%02d\r\n", 
                g_alarmTime.AlarmTime.Hours, g_alarmTime.AlarmTime.Minutes, g_alarmTime.AlarmTime.Seconds,
                systime.Hours, systime.Minutes, systime.Seconds);
            continue;
        }
        PWMStartHandler(0, NULL, 0);
        alarmCount = 0;
        while (1) {
            if(xSemaphoreTake(semaphoreStopAlarm, ALARM_MONITOR_DELAY) == pdTRUE) {
                LOG_I("Alarm stop, as key pressed\r\n");
                break;
            }
            alarmCount++;
            if ((alarmCount * ALARM_MONITOR_DELAY) < ALARM_TIME_X_MS) {
            }else{
                LOG_I("Alarm stop, as alarm time out\r\n");
                break;
            }
        }
        PWMStopHandler(0, NULL, 0);
    }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    xSemaphoreGiveFromISR(semaphoreNeedAlarm, NULL);
}

void Alarm_keyCallBackStopAlarm(void)
{
    xSemaphoreGive(semaphoreStopAlarm);
}
static void Alarm_init(void)
{
    xTaskCreate(Alarm_Task, "Alarm", 128 * 2, NULL, 25, NULL);
    LOG_D("Alarm task creat success\r\n");
    key_handlerRegister(GPIO_KEY1, Alarm_keyCallBackStopAlarm, NULL);
}

CoreInitCall(Alarm_init);
