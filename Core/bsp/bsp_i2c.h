/*!

    Copyright (c) 2020, ZK Inc.

    All rights reserved.
*/

#ifndef BSP_I2C_H
#define BSP_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

// 调试用，测试总线和驱动是事正常
#define I2C_DEV_EEPROM_ADDR			0xA0
// 对方的地址
#define I2C_DEV_IPMB_ADDR			0x20
// 本机地址
#define I2C_OWN_ADDRESS             (0x32<<0)


void i2c_channel_init(uint32_t i2cx);
void i2c_int(void);
bool i2c_write(uint32_t bus, const uint8_t *p_buffer, uint16_t len);
bool i2c_read(uint32_t bus, uint8_t devAddr, uint32_t regAddress, uint8_t regAddrLen, uint8_t *pReadBuf, uint16_t size);

bool i2c_write_bytes(uint8_t devAddr, const uint8_t *pWriteBuf, uint16_t writeSize);

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */

