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
#include "bsp_i2cs0.h"
#include "bsp_i2cs1.h"
#include "bsp_gpio.h"
#include "bsp_i2ccomm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


void Delay_NoSchedue(uint32_t clk)
{
    for (uint32_t i = 0; i < clk; i++) {
        ;
    }
}

BOOL I2C_Tx(I2C_BUS_NUM bus, INT8U devAddr, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len)
{
    switch (bus)
    {
        case I2C_BUS_S0:
            return i2cs0_write_bytes(devAddr, subaddr, sizeOfSubAddr, pWriteData, len);
        case I2C_BUS_S1:
            return i2cs1_write_bytes(devAddr, subaddr, sizeOfSubAddr, pWriteData, len);
        default:
            break;
    }
    return false;
}
BOOL I2C_Rx(I2C_BUS_NUM bus, INT8U devAddr, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len)
{
    switch (bus)
    {
        case I2C_BUS_S0:
            return i2cs0_read_bytes(devAddr, subaddr, sizeOfSubAddr, pReadData, len);
        case I2C_BUS_S1:
            return i2cs1_read_bytes(devAddr, subaddr, sizeOfSubAddr, pReadData, len);
        default:
            break;
    }
    return false;
}




