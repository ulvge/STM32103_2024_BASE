#include <string.h>   
#include <stdarg.h> 
#include "main.h"
#include "Types.h"
#include "bsp_gpio.h"
#include "shell.h"
#include "debug_print.h"

#define GPIO_GROUP_START  GPIO_DAC_A

#define STR(x) #x
#define EXPAND(x) STR(x)
#define PIN_NAME(val) .Name = EXPAND(val), .Pin = val##_PIN

//硬件上是接的这两个引脚，用来访问eeprom，但是，引脚却不是硬件I2C，只能用模拟I2C来访问
//把这两个引脚配置成浮空输入，把另外真正的硬件I2C PB6、PB7 和这两个引脚分别对接，这样就可以用硬件I2C访问eeprom了
const static GPIO_InitTypeDef g_gpioConfigComm[] = {
    {GPIOA,  "SCL", GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
    {GPIOA,  "SDA", GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
	
    //{GPIOB,  "SCL", GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
    //{GPIOB,  "SDA", GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_PIN_RESET},
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

bool GPIO_getPinName(GPIO_Idex idex, const char **name)
{
    if (idex >= ARRARY_SIZE(g_gpioConfigComm))
    {
        return false;
    }
    *name = g_gpioConfigComm[idex].Name;
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

