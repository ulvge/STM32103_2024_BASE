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
#include "bsp_gpio.h"
#include "bsp_i2ccomm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

BOOL I2C_Tx(I2C_BUS_NUM bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len)
{
    switch (bus)
    {
        case I2C_BUS_1:
            //return I2C1_Tx(dest_add, subaddr, sizeOfSubAddr, pWriteData, len);
            break;
        case I2C_BUS_S0:
            return i2cs0_write_bytes(dest_add, subaddr, sizeOfSubAddr, pWriteData, len);
        default:
            break;
    }
    return false;
}
BOOL I2C_Rx(I2C_BUS_NUM bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len)
{
    switch (bus)
    {
        case I2C_BUS_1:
            //return I2C1_Rx(dest_add, subaddr, sizeOfSubAddr, pReadData, len);
            break;
        case I2C_BUS_S0:
            return i2cs0_read_bytes(dest_add, subaddr, sizeOfSubAddr, pReadData, len);
        default:
            break;
    }
    return false;
}




