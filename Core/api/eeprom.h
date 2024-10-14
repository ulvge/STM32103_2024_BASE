#ifndef __EEPROM_H
#define	__EEPROM_H

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include <stdbool.h>
#include "Types.h"

#define AT24C01         127  // 1K bit, 128 byte, 16 pages of 8 bytes each
#define AT24C02         255  // 32 pages of 8 bytes each
#define AT24C04         511  // 32 pages of 16 bytes each
#define AT24C08         1023 // 64 pages of 16 bytes
#define AT24C16         2047 // 128 pages of 16 bytes
#define AT24C32         4095
#define AT24C64         8191
#define AT24C128         16383
#define AT24C256         32767

// ******************* 选择 bus *******************
//#define  EEP_BUS I2C_BUS_1 // 开发板
#define  EEP_BUS    I2C_BUS_S0 // printer

// ******************* 选择 型号 *******************
/* 打印机使用的是 AT24C04 */
//#define EE_TYPE             AT24C04 // printer
#define EE_TYPE             AT24C02 // 野火开发板



#define EE_ADDR_LEN         ((EE_TYPE > AT24C16) ? 2 : 1)
#define EEPROM_PAGE_BYTES   ((EE_TYPE <= AT24C02) ? 8 : 16)



#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H */

