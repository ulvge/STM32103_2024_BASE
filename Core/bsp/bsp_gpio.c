#include <string.h>   
#include <stdarg.h> 
#include "main.h"
#include "Types.h"
#include "bsp_gpio.h"
#include "shell.h"
#include "debug_print.h"
#include "pwm.h"

#define GPIO_GROUP_START  GPIO_DAC_A

#define STR(x) #x
#define EXPAND(x) STR(x)
#define PIN_NAME(val) .Name = EXPAND(val), .Pin = val##_PIN


const static GPIO_InitTypeDef g_gpioConfigComm[] = {
    {PWM_GPIO,  "PWMAF", PWM_PIN, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
    {PWM_GPIO,  "PWMPP", PWM_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
    {GPIOA,  "KEY1", GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_PIN_SET},
    {GPIOC,  "KEY2", GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_PIN_SET},
    {GPIOC,  "LED1", GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_PIN_RESET}, //硬件PA8(PWM)连接到PC2(LED1)
    {GPIOC,  "LED2", GPIO_PIN_3, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_PIN_RESET},

};
static void GPIO_InitGPIOs(const GPIO_InitTypeDef *config, uint32_t size)
{
    const GPIO_InitTypeDef *p_gpioCfg;
    if (config == NULL) {
        return;
    }
    for (uint32_t i = 0; i < size; i++)
    {
        p_gpioCfg = &config[i];
        HAL_GPIO_Init(p_gpioCfg->PORT, (GPIO_InitTypeDef *)p_gpioCfg);
		uint32_t st = !p_gpioCfg->ActiveSignal;
        HAL_GPIO_WritePin(p_gpioCfg->PORT, p_gpioCfg->Pin, (GPIO_PinState)st);
    }
}
void GPIO_ReInitGPIO(GPIO_Idex idex)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return;
    }
    const GPIO_InitTypeDef *p_gpioCfg = &g_gpioConfigComm[idex];
    HAL_GPIO_Init(p_gpioCfg->PORT, (GPIO_InitTypeDef *)p_gpioCfg);
    uint32_t st = !p_gpioCfg->ActiveSignal;
    HAL_GPIO_WritePin(p_gpioCfg->PORT, p_gpioCfg->Pin, (GPIO_PinState)st);
}
/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/// @brief return gpio hi or low
/// @param idex 
/// @return 
GPIO_PinState GPIO_getPinStatus(GPIO_Idex idex)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return GPIO_PIN_RESET;
    }
    const GPIO_InitTypeDef *p_gpioCfg = &g_gpioConfigComm[idex];
    return HAL_GPIO_ReadPin(p_gpioCfg->PORT, p_gpioCfg->Pin);
}
int GPIO_isPinActive(GPIO_Idex idex, GPIO_PinState *config)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return -1;
    }
    const GPIO_InitTypeDef *p_gpioCfg = &g_gpioConfigComm[idex];
    if (config != NULL){
        *config = p_gpioCfg->ActiveSignal;
    }
    GPIO_PinState staus = HAL_GPIO_ReadPin(p_gpioCfg->PORT, p_gpioCfg->Pin);
    if (staus == p_gpioCfg->ActiveSignal) {
        return 1;
    }
    return 0;
}

bool GPIO_getPinName(GPIO_Idex idex, char **name)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return false;
    }
    
    const GPIO_InitTypeDef *p_gpioCfg = &g_gpioConfigComm[idex];
    *name = p_gpioCfg->Name;
    return true;
}
void GPIO_printIdexAndName(void)
{
    LOG_RAW("GPIO table: idx    |     name\r\n");
    for (uint32_t i = 0; i < ARRARY_SIZE(g_gpioConfigComm); i++)
    {
        LOG_RAW("%14d         %s\r\n", i, g_gpioConfigComm[i].Name);
    }
}
/// @brief ActiveSignal  isActive  res
//          1               1      1
//          1               0      0
//          0               0      1
//          0               1      0

/// @param alias 
/// @param isActive 
/// @return 
bool GPIO_setPinStatus(GPIO_Idex idex, FunctionalState isActive, GPIO_PinState *config)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return false;
    }
    const GPIO_InitTypeDef *p_gpioCfg = &g_gpioConfigComm[idex];

    if ((p_gpioCfg->Mode < GPIO_MODE_OUTPUT_PP) || (p_gpioCfg->Mode > GPIO_MODE_AF_OD))
    {
        return false;
    }

    if (config != NULL){
        *config = p_gpioCfg->ActiveSignal;
    }
    HAL_GPIO_WritePin(p_gpioCfg->PORT, p_gpioCfg->Pin, (GPIO_PinState)(p_gpioCfg->ActiveSignal == (GPIO_PinState)isActive));
    return true;
}


void GPIO_Init(void)
{
    MX_GPIO_Init();
    
    GPIO_InitGPIOs(&g_gpioConfigComm[0], ARRARY_SIZE(g_gpioConfigComm));
}

