#include "bsp_gpio.h"
#include "Types.h"
#include "bsp_fsm.h"
#include "debug_print.h"
#include "main.h"
#include "bsp_key.h"
#include "task.h"
#include "FreeRTOS.h"
#include "initcall.h"
#include "alarm.h"

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

static Key_ScanST g_keyConfig[] = {
    {.pin = GPIO_KEY1},
    {.pin = GPIO_KEY2},
};
#define DEV_TASK_DELAY_XMS  10
static void KeyTaskHandler(void *pArg)
{
    char *keyName;
    while (1)
    {
        vTaskDelay(DEV_TASK_DELAY_XMS);
        for (UINT8 i = 0; i < ARRARY_SIZE(g_keyConfig);  i++) {
            Key_ScanST *key = &g_keyConfig[i];
            KeyPressedDurationMs(key);
            if (key->isPreesed) {
                if (GPIO_getPinName(key->pin, &keyName)){
                    LOG_I("key %s pressed\r\n", keyName);
                }
                if (key->onPressed){
                    key->onPressed();
                }
            }
            if (key->isReleased) {
                if (GPIO_getPinName(key->pin, &keyName)){
                    LOG_I("key %s released\r\n", keyName);
                }
                if (key->onReleased){
                    key->onReleased();
                }
            }
        }
    }
}
bool key_handlerRegister(GPIO_Idex idx, KeyPressedHandler onPressed, KeyReleasedHandler onReleased)
{
    for (UINT8 i = 0; i < ARRARY_SIZE(g_keyConfig);  i++) {
        Key_ScanST *key = &g_keyConfig[i];
        if (key->pin == idx) {
            key->onPressed = onPressed;
            key->onReleased = onReleased;
            return true;
        }
    }
    return false;
}
bool key_handlerUnRegister(GPIO_Idex idx)
{
    for (UINT8 i = 0; i < ARRARY_SIZE(g_keyConfig);  i++) {
        Key_ScanST *key = &g_keyConfig[i];
        if (key->pin == idx) {
            key->onPressed = NULL;
            key->onReleased = NULL;
            return true;
        }
    }
    return false;
}
static void Key_init(void)
{
    xTaskCreate(KeyTaskHandler, "DevTask", 128 * 4, NULL, 10, NULL);
}


CoreInitCall(Key_init);


