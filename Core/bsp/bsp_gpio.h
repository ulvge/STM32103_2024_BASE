#ifndef __BSP_GPIO_H
#define	__BSP_GPIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>  
#include "stm32f1xx.h"

// #define KEY0_PORT             GPIOA
// #define KEY0_PIN              GPIO_PIN_0

// #define KEY1_PORT                   GPIOC
// #define KEY1_PIN                    GPIO_PIN_13
 
typedef enum
{
    GPIO_SCL = 0U,
    GPIO_SDA,
    GPIO_PWM_AF,
    GPIO_PWM_PP,
    GPIO_KEY1,
    GPIO_KEY2,
    GPIO_LED1,
    GPIO_LED2,
    GPIO_MAX,
} GPIO_Idex;

extern void GPIO_Init(void);

inline extern void GPIO_SetDAC(uint16_t val);

void GPIO_printIdexAndName(void);
int GPIO_isPinActive(GPIO_Idex idex, GPIO_PinState *config);
bool GPIO_setPinStatus(GPIO_Idex idex, FunctionalState isActive, GPIO_PinState *config);
bool GPIO_getPinName(GPIO_Idex idex, char **name);
void GPIO_ReInitGPIO(GPIO_Idex idex);
#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */
