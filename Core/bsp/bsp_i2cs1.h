#ifndef _BSP_I2CS1_H
#define _BSP_I2CS1_H

#include <stdint.h>
#include <stdbool.h>

/********************* GPIO SIMULATED I2C1 MICRO DEF ***************************/
#define I2CS1_SCL_GPIO_PORT     GPIOA
#define I2CS1_SCL_CLK_EN        __HAL_RCC_GPIOA_CLK_ENABLE
#define I2CS1_SCL_PIN           GPIO_PIN_2

#define I2CS1_SDA_GPIO_PORT     GPIOA
#define I2CS1_SDA_CLK_EN         __HAL_RCC_GPIOA_CLK_ENABLE
#define I2CS1_SDA_PIN           GPIO_PIN_3

#define I2CS1_RETRY_TIMERS      3
/********************* GPIO SIMULATED I2C1 MICRO DEF END***************************/


/********************* GPIO SIMULATED I2C2 MICRO DEF END***************************/

/* simulated i2c1 function declarations */
void i2cs1_init(void);
bool i2cs1_read_bytes(uint8_t devAddr, uint32_t regAddress, uint8_t regAddrLen, uint8_t *_pReadBuf, uint16_t readSize);
bool i2cs1_write_bytes(uint8_t devAddr, INT32U subaddr, INT8U sizeOfSubAddr, const uint8_t *pWriteBuf, uint16_t writeSize);

void i2cs1_set_address(uint8_t _Address);
void i2cs1_Stop(void);

#endif
