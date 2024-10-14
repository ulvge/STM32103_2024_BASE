/*!
    \file    eeprom.c.c
    \brief

    \version
*/
#include <stdlib.h>
#include "Types.h"
#include "debug_print.h"
#include "eeprom.h"
#include "FreeRTOS.h"
#include "bsp_i2ccomm.h"
#include "string.h"
#include "task.h"
#include "shell_ext.h"
#include "shell_port.h"


INT8U I2C1_EEPROM_ADDR = 0xA0;

BOOL EEP_ReadData(INT32U subaddr, INT8U *pReadData, INT16U len)
{
    INT16U i, offset = 0;
    for (i = 0; i <= (len - EEPROM_PAGE_BYTES); i += EEPROM_PAGE_BYTES) {
        if (I2C_Rx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + offset, EE_ADDR_LEN, pReadData + offset, EEPROM_PAGE_BYTES) == false){
            return false;
        }
        offset += EEPROM_PAGE_BYTES;
        vTaskDelay(1);
    }
    if (len % EEPROM_PAGE_BYTES) {
        if (I2C_Rx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + offset, EE_ADDR_LEN, pReadData + offset, len % EEPROM_PAGE_BYTES) == false){
            return false;
        }
        vTaskDelay(1);
    }
    return TRUE;
}

INT8U EEP_WriteData(INT32U subaddr, INT8U *pWriteData, INT16U len)
{
    INT16U i, OffSet, Len1;
    // 1、从起始位置到页倍数
    if (len == 0)
        return TRUE;

    Len1 = EEPROM_PAGE_BYTES - (subaddr % EEPROM_PAGE_BYTES);
    if (Len1) {
        if (len <= Len1) {
            return I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr, EE_ADDR_LEN, pWriteData, len);
        } else{
            if (I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr, EE_ADDR_LEN, pWriteData, Len1) == false){
                return false;
            }
        }
        len -= Len1;
        vTaskDelay(1);
    }

    // 2、所有页倍数
    OffSet = Len1;
    for (i = 0; i < (len / EEPROM_PAGE_BYTES); i++) {
        if (I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + OffSet, EE_ADDR_LEN, pWriteData + OffSet, EEPROM_PAGE_BYTES) == false){
            return false;
        }
        OffSet += EEPROM_PAGE_BYTES;
        vTaskDelay(1);
    }
    // 3、最后的余数
    if (len % EEPROM_PAGE_BYTES) {
        return I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + OffSet, EE_ADDR_LEN, pWriteData + OffSet, len % EEPROM_PAGE_BYTES);
    }
    return TRUE;
}

static INT32U ee_paraCovert(char *input, INT32S radix)
{
    INT32U res = strtol(input, NULL, radix);
    return res;
}

static INT8U EEPROM_Buf[1024];

static void eeUsage(void)
{
    //        ee  r/w   addr    len/data
    LOG_RAW("eg: ee 0xa0 r 1 100: read from addr = 1, len = 100\n");
    LOG_RAW("eg: ee 0xa0 w 2 0x10 0x20 0x30 : write to addr = 2, data [0x10 0x20 0x30]\n");
}
#define  EEPROM_TITLE_ROWS_LEN  16
static int cmd_EEPROM(int argc, char *argv[])
{
    char IsR_W;
    INT32U devAddr, devAddrBak, regAddr, len;
    INT32U paraOffset = 1;
    
    if (argc < 5) {
        LOG_RAW("ee para need more than 5\n");
        eeUsage();
        return 0;
    }
    devAddr = ee_paraCovert(argv[paraOffset++], 0);
    IsR_W = *argv[paraOffset++];
    regAddr = ee_paraCovert(argv[paraOffset++], 0);
    
    devAddrBak = I2C1_EEPROM_ADDR;
    I2C1_EEPROM_ADDR = devAddr;
    switch (IsR_W) {
    case '0':
    case 'r':
    case 'R':
        len = ee_paraCovert(argv[paraOffset++], 0);
        if (len > sizeof(EEPROM_Buf)){
            len = sizeof(EEPROM_Buf);
        }
        if (EEP_ReadData(regAddr, EEPROM_Buf, len) == false){
            LOG_RAW("dev no ack\n");
            goto exit;
        }
        // print head row
        LOG_RAW("      ");
        // 0 1 2 3 ` 15
        for (INT32U i = 0; i < EEPROM_TITLE_ROWS_LEN; i++){
            LOG_RAW("%02d ", i);
        }
        LOG_RAW("\r\n");
        for (INT32U i = 0; i < len; i++){
            // print head column
            if ((i == 0) ||(i+regAddr) % EEPROM_TITLE_ROWS_LEN == 0){
                INT32U column = (i+regAddr + 1) / EEPROM_TITLE_ROWS_LEN;
                LOG_RAW("\r\n%02x    ", column * EEPROM_TITLE_ROWS_LEN);
                vTaskDelay(5);
            }
            if (i == 0){ // if not from 0,need alignment
                for (INT32U j = 0; j < regAddr % EEPROM_TITLE_ROWS_LEN; j++){
                    LOG_RAW("    "); // place holder
                }
            }
            LOG_RAW("%02x ", EEPROM_Buf[i]);
        }
        break;
    case '1':
    case 'w':
    case 'W':
        for (INT32S i = 0; i < argc - paraOffset; i++){
            INT32U tmp;
            tmp = ee_paraCovert(argv[paraOffset + i], 16);
            EEPROM_Buf[i] = tmp;
        }
        if (EEP_WriteData(regAddr, EEPROM_Buf, argc - paraOffset) == TRUE){
            LOG_RAW("write success\r\n");
        }else{
            LOG_RAW("write failed\r\n");
        }
        break;
        default:
            eeUsage();
            break;
    }
    exit:
    I2C1_EEPROM_ADDR = devAddrBak;
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, ee, cmd_EEPROM, read write eeprom);



