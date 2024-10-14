#ifndef _BSP_I2CCOMM_H
#define _BSP_I2CCOMM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    I2C_BUS_0 = 0,
    I2C_BUS_1,
    I2C_BUS_2,

    I2C_BUS_S0,
    I2C_BUS_S1,
    I2C_BUS_S2,
} I2C_BUS_NUM;

BOOL I2C_Tx(I2C_BUS_NUM bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len);
BOOL I2C_Rx(I2C_BUS_NUM bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len);

#endif
