/*!

    Copyright (c) 2020, ZK Inc.

    All rights reserved.
*/

#include "bsp_i2c.h"
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "task.h"



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
void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c)
{
    uint32_t itsources                = READ_REG(hi2c->Instance->CR2);
    uint32_t CurrentXferOptions       = hi2c->XferOptions;
    HAL_I2C_ModeTypeDef CurrentMode   = hi2c->Mode;
    HAL_I2C_StateTypeDef CurrentState = hi2c->State;
    
    /* Now time to read SR2, this will clear ADDR flag automatically */
    uint32_t sr2itflags   = READ_REG(hi2c->Instance->SR2);
    uint32_t sr1itflags   = READ_REG(hi2c->Instance->SR1);
    
    /* ADDR set --------------------------------------------------------------*/
    if ((I2C_CHECK_FLAG(sr1itflags, I2C_FLAG_ADDR) != RESET) && (I2C_CHECK_IT_SOURCE(itsources, I2C_IT_EVT) != RESET))
    {
        /* Now time to read SR2, this will clear ADDR flag automatically */
        if (hi2c->ErrorCode != HAL_I2C_ERROR_NONE)
        {
            sr2itflags   = READ_REG(hi2c->Instance->SR2);
        }
        //__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ADDR); //read only
        __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
		return;
    }
    /* RXNE set and BTF reset ----------------------------------------------*/
    if ((I2C_CHECK_FLAG(sr1itflags, I2C_FLAG_RXNE) != RESET) && (I2C_CHECK_IT_SOURCE(itsources, I2C_IT_BUF) != RESET) && (I2C_CHECK_FLAG(sr1itflags, I2C_FLAG_BTF) == RESET))
    {
        //I2C_SlaveReceive_RXNE(hi2c);
        HAL_I2C_SlaveRxCpltCallback(hi2c);
    }
	sr1itflags   = READ_REG(hi2c->Instance->SR1);
    //if ((I2C_CHECK_FLAG(sr1itflags, I2C_FLAG_STOPF) != RESET) && (I2C_CHECK_IT_SOURCE(itsources, I2C_IT_EVT) != RESET))
    if ((I2C_CHECK_FLAG(sr1itflags, I2C_FLAG_STOPF) != RESET))
    {
        //__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF); //read only
        __HAL_I2C_CLEAR_STOPFLAG(hi2c);
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

    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    if(HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)devAddr, (uint8_t*)pWriteBuf, writeSize, writeSize)!= HAL_OK)
    //if(HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)devAddr, (uint8_t*)pWriteBuf, writeSize)!= HAL_OK)
    {
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
        return false;
    }
    //while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	return true;
}
