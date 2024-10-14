#include "bsp_gpio.h"
#include "Types.h"
#include "dev_fsm.h"
#include "debug_print.h"
#include "main.h"
#include "bsp_key.h"
#include "task.h"
#include "FreeRTOS.h"
#include "initcall.h"

/// @brief count pluse ms of the pin
/// @return The duration of the key pressed
UINT32 KeyPressedDurationMs(Key_ScanST *key)
{
    #define KEY_JITTER_DELAY  20
    uint32_t durationMs;
    GPIO_Idex pin = key->pin;
    if (GPIO_isPinActive(pin, NULL)) {
        key->isLastPressed = true;
        durationMs = HAL_GetTick() - key->preesedStartTick;
        if (durationMs >= KEY_JITTER_DELAY) {
            key->isPreesed = true;
        }
        return durationMs;
    }else{
        if (key->isLastPressed == true) {
            key->isLastPressed = false; 
            key->isPreesed = false;
            key->isReleased = true;
            durationMs = HAL_GetTick() - key->preesedStartTick;
        } else{
            key->isReleased = false;
			durationMs = 0;
        }
        key->preesedStartTick = HAL_GetTick(); //update to newest tick
        return durationMs;
    }
}


static Key_ScanST g_key1 = {
    .pin = GPIO_KEY1
}; 
//static Key_ScanST g_key2= {
//    .pin = GPIO_KEY2
//};
#define DEV_TASK_DELAY_XMS  10
static void DevTaskHandler(void *pArg)
{
    char *key1Name;
    while (1)
    {
        vTaskDelay(DEV_TASK_DELAY_XMS);
        KeyPressedDurationMs(&g_key1);

        if (g_key1.isReleased) {
            GPIO_setPinStatus(GPIO_LED2, ENABLE, NULL);
            if (GPIO_getPinName(g_key1.pin, &key1Name)){
                LOG_I("key %s pressed\r\n", key1Name);
            }
        }
    }
}

static void Key_init(void)
{
    xTaskCreate(DevTaskHandler, "DevTask", 128 * 4, NULL, 10, NULL);
}


CoreInitCall(Key_init);


