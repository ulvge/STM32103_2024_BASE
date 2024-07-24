/*!

    Copyright (c) 2020, ZK Inc.

    All rights reserved.
*/

#include "bsp_i2c.h"
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "task.h"


#define I2C_OWN_ADDRESS  0x32

#define I2C1_INTERRUPT_ENALBE
#define I2C_CLOCK_100K  100000U
#define I2C_CLOCK_400K  400000U

static void i2c1_int(void);

//static bool i2c_bytes_write(uint32_t i2cx, uint8_t device_addr, const uint8_t *p_buffer, uint16_t len, int time_out);

void i2c_int(void)
{
    i2c1_int();
}

extern I2C_HandleTypeDef hi2c1;
static void i2c1_int(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = I2C_CLOCK_400K;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = I2C_OWN_ADDRESS;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0xFF;
    //hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_ENABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) // HAL_I2C_MspInit
    {
        Error_Handler();
    }
}

bool i2c_write_bytes(uint8_t devAddr, const uint8_t *pWriteBuf, uint16_t writeSize)
{
    #define I2C_WAIT_FOR_IDLE 2
    #define I2C_TIME_OUT 20
    uint32_t waitIdleCount = 0;
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
        if (waitIdleCount++ >= I2C_TIME_OUT / I2C_WAIT_FOR_IDLE){
            return false;
        }
        vTaskDelay(I2C_WAIT_FOR_IDLE);
    }

    if(HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)devAddr, (uint8_t*)pWriteBuf, writeSize)!= HAL_OK)
    {
        return false;
    }
    //while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
	return true;
}
