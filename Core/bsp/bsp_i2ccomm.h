#ifndef _BSP_I2CCOMM_H
#define _BSP_I2CCOMM_H

#include <stdint.h>
#include <stdbool.h>

#define I2CS_WRITE 0 /* write ctrl bit */
#define I2CS_READ 1 /* read ctrl bit */

typedef enum
{
    I2C_BUS_0 = 0,
    I2C_BUS_1,
    I2C_BUS_2,

    I2C_BUS_S0,
    I2C_BUS_S1,
    I2C_BUS_S2,
} I2C_BUS_NUM;

void Delay_NoSchedue(uint32_t clk);
BOOL I2C_Tx(I2C_BUS_NUM bus, INT8U devAddr, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len);
BOOL I2C_Rx(I2C_BUS_NUM bus, INT8U devAddr, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len);

#endif
