
#include "includes.h"
#include "i2c.h"

INT8U I2C1_EEPROM_ADDR = 0xA0;
#if DBG_I2C
#define I2C_DEBUG(X) \
    do {             \
        DPrint X;    \
    } while (0)
#else
#define I2C_DEBUG(X)
#endif

//*****************************************************
//		I2C1
//*****************************************************
GPIO_InitTypeDef GPIO_SDA_In_I2C1, GPIO_SDA_Out_I2C1;
/*I2C1  driver***********************************************************************************/
static INT8U I2C1_Wait_MinCnt = 6;
void I2C1_WaitTick(void)
{
    INT8U dly = I2C1_Wait_MinCnt; // EepMin=5;7419Min=1;
    while (dly--) {
    }
}

void I2C1_I2cStart(void) i2cs0_Start
{
    // GPIO_SetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();
}

void I2C1_I2cStop(void) i2cs0_Stop
{
    GPIO_ResetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();
}

void I2C1_I2CAck(void) i2cs0_Ack
{
    GPIO_ResetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
    I2C1_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
}
void I2C1_I2CNAck(void) i2cs0_NAck
{
    GPIO_SetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
    I2C1_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
}

BOOL I2C1_I2CTxByte(INT8U Data)  i2cs0_SendByte
{
    INT8U cnt = 8;
    BOOL ret = FALSE;

    while (cnt--) {
        GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
        I2C1_WaitTick();

        if (Data & 0x80)
            GPIO_SetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
        else
            GPIO_ResetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);

        Data = (INT8U)(Data << 1);
        I2C1_WaitTick();

        GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
        I2C1_WaitTick();
    }

    GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();

    GPIO_Init(IO_PORT_I2C1_SDA_DET, &GPIO_SDA_In_I2C1);
    I2C1_WaitTick();

    GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    I2C1_WaitTick();
    I2C1_WaitTick();
    if (GPIO_ReadInputDataBit(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET) == 0) {
        ret = TRUE;
    }

    //GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
    //I2C1_WaitTick();

    GPIO_Init(IO_PORT_I2C1_SDA_DET, &GPIO_SDA_Out_I2C1);
    I2C1_WaitTick();

    return ret;
}

INT8U I2C1_I2CRxByte(void)
{
    INT8U cnt = 8;
    INT8U ReadData = 0;

    GPIO_Init(IO_PORT_I2C1_SDA_DET, &GPIO_SDA_In_I2C1);

    while (cnt--) {
        ReadData = (INT8U)(ReadData << 1);

        GPIO_SetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
        I2C1_WaitTick();
        I2C1_WaitTick();

        if (GPIO_ReadInputDataBit(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET) != 0) {
            ReadData |= 0x01;
        }

        GPIO_ResetBits(IO_PORT_I2C1_SCL_DET, IO_PIN_I2C1_SCL_DET);
        I2C1_WaitTick();
        I2C1_WaitTick();
    }

    GPIO_Init(IO_PORT_I2C1_SDA_DET, &GPIO_SDA_Out_I2C1);
    return ReadData;
}

BOOL I2C_Tx(INT8U bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len)
{
    if (bus == 1) {
        return I2C1_Tx(dest_add, subaddr, sizeOfSubAddr, pWriteData, len);
    }else{
        return I2C2_Tx(dest_add, subaddr, sizeOfSubAddr, pWriteData, len);
    }
}
BOOL I2C_Rx(INT8U bus, INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len)
{
    if (bus == 1) {
        return I2C1_Rx(dest_add, subaddr, sizeOfSubAddr, pReadData, len);
    }else{
        return I2C2_Rx(dest_add, subaddr, sizeOfSubAddr, pReadData, len);
    }
}
BOOL I2C1_Tx(INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len)
{
    INT8U i, loop = 2, ret = false;

    while (loop--) {
        if ((len != 0) && (pWriteData != NULL)) {
            I2C1_I2cStart();
            I2C1_I2CTxByte(dest_add);
            while (sizeOfSubAddr--) {
                I2C1_I2CTxByte(subaddr & 0xff);
                subaddr >>= 8;
            }
            for (i = 0; i < len; i++) {
                ret = I2C1_I2CTxByte(pWriteData[i]);
                I2C1_WaitTick();
            }
            I2C1_I2cStop();

            if (ret == false) {
                if (loop == 0) {
#if I2C_DEBUG
                    static INT32U ErrCnt;
#endif
                    I2C_DEBUG((":> IIC send failed: %s,%l\n", __FILE__, __LINE__));
                    I2C_DEBUG((":> IIC send failed: WriteData: [%h]\n", pWriteData, len));
                    I2C_DEBUG((":> IIC send failed: dest_add:0x%o,subaddr:%x\n", dest_add, subaddr));
                }
            } else
                break;
        }
    }
    return ret;
}
BOOL I2C1_Rx(INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len)
{
    BOOL ret = FALSE;
    INT8U i, loop = 2;

    do {
        loop--;
        I2C1_I2cStart();

        ret = I2C1_I2CTxByte(dest_add);
        if (ret) {
            while (sizeOfSubAddr--) {
                I2C1_I2CTxByte(subaddr & 0xff);
                subaddr >>= 8;
            }
            if (ret) {
                GPIO_SetBits(IO_PORT_I2C1_SDA_DET, IO_PIN_I2C1_SDA_DET);
                I2C1_WaitTick();
                I2C1_I2cStart();
                ret = I2C1_I2CTxByte(dest_add | 0X01);
                if (ret) {
                    for (i = 0; i < len; i++) {
                        pReadData[i] = I2C1_I2CRxByte();
                        if (i != (len - 1))
                            I2C1_I2CAck();
                        else
                            I2C1_I2CNAck();
                    }
                }
            }
        }
        I2C1_I2cStop();
    } while (loop > 0 && ret == FALSE);

    if (!ret) {
        I2C_DEBUG((":> IIC读取错误:%s,%l\n", __FILE__, __LINE__));
        I2C_DEBUG((":> IIC读取错误:dest_add:0x%o,subaddr:%x\n", dest_add, subaddr));
    }
    return ret;
}

void I2C1_DrvInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // Pin 92
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IO_PIN_I2C1_SCL_DET;
    GPIO_Init(IO_PORT_I2C1_SCL_DET, &GPIO_InitStructure);
    // Pin 93
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = IO_PIN_I2C1_SDA_DET;
    GPIO_Init(IO_PORT_I2C1_SDA_DET, &GPIO_InitStructure);
    I2C1_I2cStop();

    GPIO_SDA_In_I2C1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_SDA_In_I2C1.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_SDA_In_I2C1.GPIO_Pin = IO_PIN_I2C1_SDA_DET;

    GPIO_SDA_Out_I2C1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_SDA_Out_I2C1.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_SDA_Out_I2C1.GPIO_Pin = IO_PIN_I2C1_SDA_DET;
}

//*****************************************************
//		I2C2
//*****************************************************
static INT8U I2C2_Wait_MinCnt = 220;
void I2C2_WaitTick(void)
{
    INT8U dly = I2C2_Wait_MinCnt; // EepMin=5;7419Min=1;
    while (dly--) {
    }
}
GPIO_InitTypeDef GPIO_SDA_In_I2C2, GPIO_SDA_Out_I2C2;
/*I2C2  driver***********************************************************************************/

void I2C2_I2cStart(void)
{
    // GPIO_SetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();
}

void I2C2_I2cStop(void)
{
    GPIO_ResetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();
}

void I2C2_I2CAck(void)
{
    GPIO_ResetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
    I2C2_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
}
void I2C2_I2CNAck(void)
{
    GPIO_SetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
    I2C2_WaitTick();
    GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
}

BOOL I2C2_I2CTxByte(INT8U Data)
{
    INT8U cnt = 8;
    BOOL ret = FALSE;

    while (cnt--) {
        GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
        I2C2_WaitTick();

        if (Data & 0x80)
            GPIO_SetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
        else
            GPIO_ResetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);

        Data = (INT8U)(Data << 1);
        I2C2_WaitTick();

        GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
        I2C2_WaitTick();
    }

    GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();

    GPIO_Init(IO_PORT_I2C2_SDA_DET, &GPIO_SDA_In_I2C2);
    I2C2_WaitTick();

    GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();
    I2C2_WaitTick();
    if (GPIO_ReadInputDataBit(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET) == 0) {
        ret = TRUE;
    }

    GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
    I2C2_WaitTick();

    GPIO_Init(IO_PORT_I2C2_SDA_DET, &GPIO_SDA_Out_I2C2);
    I2C2_WaitTick();

    return ret;
}

INT8U I2C2_I2CRxByte(void)
{
    INT8U cnt = 8;
    INT8U ReadData = 0;

    GPIO_Init(IO_PORT_I2C2_SDA_DET, &GPIO_SDA_In_I2C2);

    while (cnt--) {
        ReadData = (INT8U)(ReadData << 1);

        GPIO_SetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
        I2C2_WaitTick();
        I2C2_WaitTick();

        if (GPIO_ReadInputDataBit(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET) != 0) {
            ReadData |= 0x01;
        }

        GPIO_ResetBits(IO_PORT_I2C2_SCL_DET, IO_PIN_I2C2_SCL_DET);
        I2C2_WaitTick();
        I2C2_WaitTick();
    }

    GPIO_Init(IO_PORT_I2C2_SDA_DET, &GPIO_SDA_Out_I2C2);
    return ReadData;
}

BOOL I2C2_Tx(INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pWriteData, INT16U len)
{
    INT16U i, loop = 2, ret = false;

    // I2C_DEBUG((":> IIC??: dest_add: [%o],subaddr:[%o],WriteData: [%h]\n",dest_add,subaddr,pWriteData,len));
    while (loop--) {
        if ((len != 0) && (pWriteData != NULL)) {
            I2C2_I2cStart();
            I2C2_I2CTxByte(dest_add);
            while (sizeOfSubAddr--) {
                I2C2_I2CTxByte(subaddr & 0xff);
                subaddr >>= 8;
            }
            for (i = 0; i < len; i++) {
                ret = I2C2_I2CTxByte(pWriteData[i]);
                I2C2_WaitTick();
            }
            I2C2_I2cStop();

            if (ret == false) {
                if (loop == 0) {
#if I2C_DEBUG
                    static INT32U ErrCnt;
#endif
                    I2C_DEBUG((":> IIC send failed: %s,%l\n", __FILE__, __LINE__));
                    I2C_DEBUG((":> IIC send failed: WriteData: [%h]\n", pWriteData, len));
                    I2C_DEBUG((":> IIC send failed: dest_add:0x%o,subaddr:%x\n", dest_add, subaddr));
                }
            } else
                break;
        }
    }
    return ret;
}
BOOL I2C2_Rx(INT8U dest_add, INT32U subaddr, INT8U sizeOfSubAddr, INT8U *pReadData, INT16U len)
{
    BOOL ret = FALSE;
    INT16U i, loop = 2;

    do {
        loop--;
        I2C2_I2cStart();

        ret = I2C2_I2CTxByte(dest_add);
        if (ret) {
            while (sizeOfSubAddr--) {
                I2C2_I2CTxByte(subaddr & 0xff);
                subaddr >>= 8;
            }
            if (ret) {
                GPIO_SetBits(IO_PORT_I2C2_SDA_DET, IO_PIN_I2C2_SDA_DET);
                I2C2_WaitTick();
                I2C2_I2cStart();
                ret = I2C2_I2CTxByte(dest_add | 0X01);
                if (ret) {
                    for (i = 0; i < len; i++) {
                        pReadData[i] = I2C2_I2CRxByte();
                        if (i != (len - 1))
                            I2C2_I2CAck();
                        else
                            I2C2_I2CNAck();
                    }
                }
            }
        }
        I2C2_I2cStop();
    } while (loop > 0 && ret == FALSE);

    if (!ret) {
        I2C_DEBUG((":> IIC读取错误:%s,%l\n", __FILE__, __LINE__));
        I2C_DEBUG((":> IIC读取错误:dest_add:0x%o,subaddr:%x\n", dest_add, subaddr));
    }
    return ret;
}

void I2C2_DrvInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // Pin 47
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IO_PIN_I2C2_SCL_DET;
    GPIO_Init(IO_PORT_I2C2_SCL_DET, &GPIO_InitStructure);
    // Pin 48
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IO_PIN_I2C2_SDA_DET;
    GPIO_Init(IO_PORT_I2C2_SDA_DET, &GPIO_InitStructure);
    I2C2_I2cStop();

    GPIO_SDA_In_I2C2.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_SDA_In_I2C2.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_SDA_In_I2C2.GPIO_Pin = IO_PIN_I2C2_SDA_DET;

    GPIO_SDA_Out_I2C2.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_SDA_Out_I2C2.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_SDA_Out_I2C2.GPIO_Pin = IO_PIN_I2C2_SDA_DET;
}

static void I2C_Monitor(void)
{
    static int Step;
    // static INT8U * CmdWr = "test2 0 0 5";
    // static INT8U * CmdRd = "test2 1 0 5";

    if (Step == 0) { // if(FIFO_Write(&FIFO_Buf[FIFO_Chan_USART].rfifo,aubData) == false){
        Step++;
        // FIFO_Writes(&FIFO_Buf[FIFO_Chan_USART].rfifo,"test2 0 0 5",sizeof("test2 0 0 5")-1);
    } else {
        Step = 0;
        FIFO_Writes(&FIFO_Buf[FIFO_Chan_USART].rfifo, "test2 1 0 5", sizeof("test2 1 0 5") - 1);
    }

    FIFO_Write(&FIFO_Buf[FIFO_Chan_USART].rfifo, KEY_CR);
}
TIMER *I2cTmr;
void I2C_main(void)
{
    I2C1_DrvInit();
    I2C2_DrvInit();
    I2cTmr = CreateTimer(I2C_Monitor);
#if EN_DEBUG == 0
    StartTimer(I2cTmr, _MS(1500));
#endif
}
#define  EEP_BUS 1 // printer
//#define  EEP_BUS 2 // 开发板
BOOL EEP_ReadData(INT32U subaddr, INT8U *pReadData, INT16U len)
{
    INT16U i, offset = 0;
    for (i = 0; i <= (len - EEPROM_PAGE_BYTES); i += EEPROM_PAGE_BYTES) {
        if (I2C_Rx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + offset, EE_ADDR_LEN, pReadData + offset, EEPROM_PAGE_BYTES) == false){
            return false;
        }
        offset += EEPROM_PAGE_BYTES;
        RTC_DelayXms(20);
    }
    if (len % EEPROM_PAGE_BYTES) {
        if (I2C_Rx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + offset, EE_ADDR_LEN, pReadData + offset, len % EEPROM_PAGE_BYTES) == false){
            return false;
        }
        RTC_DelayXms(20);
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
        RTC_DelayXms(20);
    }

    // 2、所有页倍数
    OffSet = Len1;
    for (i = 0; i < (len / EEPROM_PAGE_BYTES); i++) {
        if (I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + OffSet, EE_ADDR_LEN, pWriteData + OffSet, EEPROM_PAGE_BYTES) == false){
            return false;
        }
        OffSet += EEPROM_PAGE_BYTES;
        RTC_DelayXms(20);
    }
    // 3、最后的余数
    if (len % EEPROM_PAGE_BYTES) {
        return I2C_Tx(EEP_BUS, I2C1_EEPROM_ADDR, subaddr + OffSet, EE_ADDR_LEN, pWriteData + OffSet, len % EEPROM_PAGE_BYTES);
    }
    return TRUE;
}
