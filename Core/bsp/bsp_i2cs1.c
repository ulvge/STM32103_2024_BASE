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
#include "string.h"
#include "bsp_i2cs1.h"
#include "bsp_i2ccomm.h"
#include "bsp_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "initcall.h"

/********************* GPIO SIMULATED I2C1 MICRO DEF ***************************/
#define I2CS1_SDA_READ() HAL_GPIO_ReadPin(I2CS1_SDA_GPIO_PORT, I2CS1_SDA_PIN) /* SDA read */
/********************* GPIO SIMULATED I2C1 MICRO DEF END***************************/

/*
*********************************************************************************************************
local function
*********************************************************************************************************
*/
static void i2cs1_CfgGpio(void);

/* simulated i2c1 function declarations */
static void      i2cs1_Start          (void);
static BOOL      i2cs1_SendByte       (uint8_t _ucByte);
static uint8_t   i2cs1_ReadByte       (void);
static void      i2cs1_Ack            (void);
static void      i2cs1_NAck           (void);
static void     i2cs1_WaitTick(INT8U dly);

static xSemaphoreHandle g_I2CS1_semaphore = NULL;
#define I2CS1_TAKE_SEMAPHORE_TIMEOUT      100

static GPIO_InitTypeDef GPIO_SDA_In_I2CS1, GPIO_SDA_Out_I2CS1;
/*
*********************************************************************************************************
*********************************************************************************************************
*/

static void i2cs1_Delay(uint32_t clk)
{
    Delay_NoSchedue(clk);
}

static void I2CS1_SCL_1(void)
{
    HAL_GPIO_WritePin(I2CS1_SCL_GPIO_PORT, I2CS1_SCL_PIN, GPIO_PIN_SET);/* SCL = 1 */
    i2cs1_Delay(30);
}
static void I2CS1_SCL_0(void)
{
    HAL_GPIO_WritePin(I2CS1_SCL_GPIO_PORT, I2CS1_SCL_PIN, GPIO_PIN_RESET);/* SCL = 0 */
    i2cs1_Delay(20);
}

static void I2CS1_SDA_1(void)
{
    HAL_GPIO_WritePin(I2CS1_SDA_GPIO_PORT, I2CS1_SDA_PIN, GPIO_PIN_SET); /* SDA = 1 */
    i2cs1_Delay(20);
}
static void I2CS1_SDA_0(void)
{
    HAL_GPIO_WritePin(I2CS1_SDA_GPIO_PORT, I2CS1_SDA_PIN, GPIO_PIN_RESET);/* SDA = 0 */
    i2cs1_Delay(20);
}
/*
*********************************************************************************************************

*********************************************************************************************************
*/
static void i2cs1_CfgGpio(void)
{
    if (g_I2CS1_semaphore == NULL)
    {
        g_I2CS1_semaphore = xSemaphoreCreateMutex();
    }
    I2CS1_SCL_CLK_EN();
    I2CS1_SDA_CLK_EN();
    GPIO_InitTypeDef gpioCfg = {0};
    gpioCfg.PORT = I2CS1_SCL_GPIO_PORT;
    gpioCfg.Pin = I2CS1_SCL_PIN;
    gpioCfg.Mode = GPIO_MODE_OUTPUT_PP;
    gpioCfg.Pull = GPIO_NOPULL;
    gpioCfg.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gpioCfg.PORT, &gpioCfg);
    
    gpioCfg.PORT = I2CS1_SDA_GPIO_PORT;
    gpioCfg.Pin = I2CS1_SDA_PIN;
    gpioCfg.Mode = GPIO_MODE_OUTPUT_OD;
    gpioCfg.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(gpioCfg.PORT, &gpioCfg);

    i2cs1_Stop();

    memcpy((INT8U *)&GPIO_SDA_Out_I2CS1, &gpioCfg, sizeof(GPIO_InitTypeDef));
    gpioCfg.Mode = GPIO_MODE_INPUT;
    memcpy((INT8U *)&GPIO_SDA_In_I2CS1, &gpioCfg, sizeof(GPIO_InitTypeDef));
}
/*
*********************************************************************************************************

*********************************************************************************************************
*/
void i2cs1_set_address(uint8_t _Address)
{
}
/*
*********************************************************************************************************

*********************************************************************************************************
*/
static void i2cs1_Start(void)
{
    I2CS1_SCL_1();
    I2CS1_SDA_0();
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
void i2cs1_Stop(void)
{
    I2CS1_SDA_0();
    I2CS1_SCL_1();
    I2CS1_SDA_1();
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
static BOOL i2cs1_SendByte(uint8_t _ucByte)
{
    uint8_t i;
    BOOL ret = FALSE;

    for (i = 0; i < 8; i++)
    {
        I2CS1_SCL_0();
        if (_ucByte & 0x80)
        {
            I2CS1_SDA_1();
        }
        else
        {
            I2CS1_SDA_0();
        }
        _ucByte <<= 1;
        I2CS1_SCL_1();
    }
    
    I2CS1_SCL_0();

    HAL_GPIO_Init(GPIO_SDA_In_I2CS1.PORT, &GPIO_SDA_In_I2CS1);

    I2CS1_SCL_1();
    if (I2CS1_SDA_READ() == 0) {
        ret = TRUE;
    }

    I2CS1_SCL_0();

    HAL_GPIO_Init(GPIO_SDA_Out_I2CS1.PORT, &GPIO_SDA_Out_I2CS1);

    return ret;
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
static uint8_t i2cs1_ReadByte(void)
{
    INT8U cnt = 8;
    INT8U ReadData = 0;

    HAL_GPIO_Init(GPIO_SDA_In_I2CS1.PORT, &GPIO_SDA_In_I2CS1);

    while (cnt--) {
        ReadData = (INT8U)(ReadData << 1);

        I2CS1_SCL_1();
        i2cs1_WaitTick(20);

        if (I2CS1_SDA_READ()) {
            ReadData |= 0x01;
        }

        I2CS1_SCL_0();
        i2cs1_WaitTick(20);
    }

    HAL_GPIO_Init(GPIO_SDA_Out_I2CS1.PORT, &GPIO_SDA_Out_I2CS1);
    return ReadData;
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
static void i2cs1_WaitTick(INT8U dly)
{
    while (dly--) {
    }
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
static void i2cs1_Ack(void)
{
    I2CS1_SDA_0();
    I2CS1_SCL_1();
    i2cs1_WaitTick(100);
    I2CS1_SCL_0();
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
static void i2cs1_NAck(void)
{
    I2CS1_SDA_1();
    I2CS1_SCL_1();
    I2CS1_SCL_0();
}

/*
*********************************************************************************************************

*********************************************************************************************************
*/
void i2cs1_init(void)
{
    i2cs1_CfgGpio();
    i2cs1_Stop();
}

/*
*********************************************************************************************************
regAddress 
low 16bit, The meaning of the name
hi 16bit: regAddressLenth,default=0, if val=2, means len=2
*********************************************************************************************************
*/
bool i2cs1_read_bytes(uint8_t devAddr, uint32_t regAddress, uint8_t regAddressLen, uint8_t *_pReadBuf, uint16_t readSize)
{
    BOOL ret = FALSE;
    INT8U i, loop = 2;
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        if (xSemaphoreTake(g_I2CS1_semaphore, I2CS1_TAKE_SEMAPHORE_TIMEOUT) == pdFALSE)
        {
            return false;
        }
    }

    do {
        loop--;
        i2cs1_Start();

        ret = i2cs1_SendByte(devAddr | I2CS_WRITE);
        if (ret) {
            while (regAddressLen--) {
                i2cs1_SendByte(regAddress & 0xff);
                regAddress >>= 8;
            }
            if (ret) {
                I2CS1_SDA_1();
                i2cs1_Start();
                ret = i2cs1_SendByte(devAddr | I2CS_READ);
                if (ret) {
                    for (i = 0; i < readSize; i++) {
                        _pReadBuf[i] = i2cs1_ReadByte();
                        if (i != (readSize - 1))
                            i2cs1_Ack();
                        else
                            i2cs1_NAck();
                    }
                }
            }
        }
    } while (loop > 0 && ret == FALSE);

    i2cs1_Stop();
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        xSemaphoreGive(g_I2CS1_semaphore);
    }
    return ret;
}
/*
*********************************************************************************************************

*********************************************************************************************************
*/
bool i2cs1_write_bytes(uint8_t devAddr, INT32U subaddr, INT8U sizeOfSubAddr, const uint8_t *pWriteBuf, uint16_t writeSize)
{
    uint16_t i, m;
    INT16U ret;
    INT32U subaddrBak = subaddr;
    INT8U sizeOfSubAddrBak = sizeOfSubAddr;
    
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        if (xSemaphoreTake(g_I2CS1_semaphore, I2CS1_TAKE_SEMAPHORE_TIMEOUT) == pdFALSE)
        {
            return false;
        }
    }

    for (m = 0; m < I2CS1_RETRY_TIMERS; m++)
    {
        i2cs1_Start();
        ret = i2cs1_SendByte(devAddr | I2CS_WRITE);

        subaddrBak = subaddr;
        sizeOfSubAddrBak = sizeOfSubAddr;
        while (sizeOfSubAddrBak--) {
            ret = i2cs1_SendByte(subaddr & 0xff);
            subaddrBak >>= 8;
        }
        if (ret != TRUE)
        {
            i2cs1_Stop();
            continue;   // retry
        }
        for (i = 0; i < writeSize; i++)
        {
            ret = i2cs1_SendByte(pWriteBuf[i]);
            if (ret != TRUE)
            {
                i2cs1_Stop();
                break;      // retry
            }
        }
        if (i == writeSize)
        {
            break;//write finished
        }
    }

    i2cs1_Stop();
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        xSemaphoreGive(g_I2CS1_semaphore);
    }
    
    if (m == I2CS1_RETRY_TIMERS)
    {
        return false;
    }else{
        return true;
    }
}


AppInitCall(i2cs1_init);



