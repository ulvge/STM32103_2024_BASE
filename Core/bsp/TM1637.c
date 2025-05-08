
#include <stdint.h>
#include <stdlib.h>
#include "stm32f1xx.h"
#include "TM1637.h"
#include "FreeRTOS.h"
#include "bsp_gpio.h"
#include "debug_print.h"
#include "initcall.h"
#include "main.h"
#include "shell.h"
#include "task.h"
#include "api_utc.h"

// 不是真正的I2C，有点像而已

#define TIM1637_REFRESH_CYCLE 500
#define TIM1637_ENCODE_OF_CLEAR 0x00
static const INT8U g_BCDOfNotPoint[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0xC0};
static const INT8U g_BCDOfPoint[] = {0xbf, 0x86, 0xdb, 0xcf, 0xe6, 0xed, 0xfd, 0x87, 0xff, 0xef, 0x40};
#define TIM1637_ENCODE_OF_ERROR 10;

void TM1637_GPIOInit(void);
void I2C_Start(void);
void I2C_stop(void);
void TM1637_WriteBit(unsigned char mBit);
void TM1637_WriteByte(unsigned char Byte);
void TM1637_WriteCommand(unsigned char mData);
void TM1637_WriteData(unsigned char addr, unsigned char mData);

static DispPara g_dispPara;

void TM1637_Delay_Us(void)
{
    INT32U dly = 2;
    while (dly--) {
    }
}
/*
 *@breif GPIO_INIT
 *@note  no
 */
void TM1637_GPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TM_DIO_CLK();
    TM_CLK_CLK();

    GPIO_InitStructure.PORT = TM_DIO_PORT;
    GPIO_InitStructure.Pin = TM_DIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIO_InitStructure.PORT, &GPIO_InitStructure);

    GPIO_InitStructure.PORT = TM_CLK_PORT;
    GPIO_InitStructure.Pin = TM_CLK_PIN;
    HAL_GPIO_Init(GPIO_InitStructure.PORT, &GPIO_InitStructure);
}

typedef enum {
    IIC_DATA_PIN_INPUT = 0,
    IIC_DATA_PIN_OUTPUT = 1
} IIC_DATA_PIN_MODE;
void TM1637_SwitchDataPinMode(IIC_DATA_PIN_MODE pinMOde)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.PORT = TM_DIO_PORT;
    GPIO_InitStructure.Pin = TM_DIO_PIN;
    if (pinMOde == IIC_DATA_PIN_INPUT) {
        GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    } else {
        GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    }
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIO_InitStructure.PORT, &GPIO_InitStructure);
}

/*
时钟线为高时，数据线由高变低
不是真正的I2C，有点像而已
*/
void I2C_Start(void)
{
    CLK_1;
    DIO_1;
    TM1637_Delay_Us();

    DIO_0;
    TM1637_Delay_Us();
    CLK_0;
    TM1637_Delay_Us();
}

/*
 *时钟线为高时，数据线由低变高
 */
void I2C_stop(void)
{
    CLK_0;
    TM1637_Delay_Us();
    DIO_0;
    TM1637_Delay_Us();

    CLK_1;
    TM1637_Delay_Us();
    DIO_1;
    TM1637_Delay_Us();
}

// 写一个比特位
void TM1637_WriteBit(unsigned char mBit)
{
    CLK_0;
    TM1637_Delay_Us();

    if (mBit) {
        DIO_1;
    } else {
        DIO_0;
    }
    TM1637_Delay_Us();
    CLK_1;
    TM1637_Delay_Us();
}

// 写一个字节
void TM1637_WriteByte(unsigned char Byte)
{
    char loop = 0;
    for (loop = 0; loop < 8; loop++) {
        TM1637_WriteBit((Byte >> loop) & 0x01); // 先写低位
    }
    CLK_0;
    TM1637_Delay_Us();
    DIO_1;
    TM1637_Delay_Us();
    CLK_1;
    TM1637_Delay_Us();
    TM1637_SwitchDataPinMode(IIC_DATA_PIN_INPUT);
    // INT32U timeOut = 30 * 1000;//2000 * 1000  1500ms ;  30 * 1000 20ms
    INT32U timeOut = 30; // 2000 * 1000  1500ms ;  30 * 1000 20ms
    // TM1637_DEBUG(("TM1637 timeout start"));
    while (HAL_GPIO_ReadPin(TM_DIO_PORT, TM_DIO_PIN) == 0x01) {
        if (timeOut-- == 0) {
            // TM1637_DEBUG(("TM1637 timeout stop"));
            break;
        }
    }
    TM1637_SwitchDataPinMode(IIC_DATA_PIN_OUTPUT);
    // 等待应答信号
}

/*
 *写指令函数
 *B7:B6 =01  数据命令控制
 *            11地址命令控制
 *            10显示命令控制
 */

void TM1637_WriteCommand(unsigned char mData)
{
    I2C_Start();
    TM1637_WriteByte(mData);
    I2C_stop();
}

/*
 *@note  写入地址和数据
 */

void TM1637_WriteData(unsigned char addr, unsigned char mData)
{
    I2C_Start();
    TM1637_WriteByte(addr);
    TM1637_WriteByte(mData);
    I2C_stop();
}
#define TM1637_DISPLAY_MODE_ADDR_AUTO 0x40
#define TM1637_DISPLAY_MODE_ADDR_MANUAL 0x44
typedef enum {
    TM1637_DISPLAY_ADDR_0 = 0xc0,
    TM1637_DISPLAY_ADDR_1 = 0xc1,
    TM1637_DISPLAY_ADDR_2 = 0xc2,
    TM1637_DISPLAY_ADDR_3 = 0xc3,
} TM1637_DISPLAY_ADDR;

typedef enum {
    TM1637_DISPLAY_PULSE_WIDTH_1DIV16 = 0x88,
    TM1637_DISPLAY_PULSE_WIDTH_2DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_4DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_10DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_11DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_12DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_13DIV16,
    TM1637_DISPLAY_PULSE_WIDTH_14DIV16 = 0x8F,
    TM1637_DISPLAY_PULSE_WIDTH_MAX,
} TM1637_DISPLAY_PULSE_WIDTH;
TM1637_DISPLAY_PULSE_WIDTH g_TM1637PluseWidth = TM1637_DISPLAY_PULSE_WIDTH_4DIV16;
__attribute__((unused)) static TM1637_DISPLAY_PULSE_WIDTH TIM1637_ChangePluseWidth()
{
    TM1637_DISPLAY_PULSE_WIDTH tmp = g_TM1637PluseWidth;
    tmp++;
    if (tmp >= TM1637_DISPLAY_PULSE_WIDTH_MAX) {
        tmp = TM1637_DISPLAY_PULSE_WIDTH_1DIV16;
    }
    g_TM1637PluseWidth = tmp;
    return g_TM1637PluseWidth;
}
static INT8U TM1637_GetLEDCode(INT8U val, BOOL isHaveDot)
{
    if (val >= sizeof(g_BCDOfNotPoint)) {
        val %= 10;
    }
    INT8U code;
    if (isHaveDot == FALSE) {
        code = g_BCDOfNotPoint[val];
    } else {
        code = g_BCDOfPoint[val];
    }
    return code;
}

static void TM1637_isBcdOfDisplayHead(INT8U disp, char *bcd, INT8U *isFirstHaveData)
{
    if (*isFirstHaveData == TRUE) {
        return;
    }
    if (disp == 0) {
        *bcd = TIM1637_ENCODE_OF_CLEAR;
    } else {
        *isFirstHaveData = TRUE;
    }
}
static void TM1637_GetBcdOfDisplay(DispContext *dispContext, DispContext *bcdContext, TM1637_DISPLAY_TYPE type, INT8U isDisplayDot)
{
    bcdContext->hour.shi = TM1637_GetLEDCode(dispContext->hour.shi, FALSE);
    if ((type == DISPLAY_TYPE_TIME) && isDisplayDot) {
        bcdContext->hour.ge = TM1637_GetLEDCode(dispContext->hour.ge, TRUE);
    } else {
        bcdContext->hour.ge = TM1637_GetLEDCode(dispContext->hour.ge, FALSE);
    }
    bcdContext->min.shi = TM1637_GetLEDCode(dispContext->min.shi, FALSE);
    bcdContext->min.ge = TM1637_GetLEDCode(dispContext->min.ge, FALSE);

    INT8U isFirstHaveData = FALSE;

    TM1637_isBcdOfDisplayHead(dispContext->hour.shi, &bcdContext->hour.shi, &isFirstHaveData);
    if (type != DISPLAY_TYPE_TIME) {
        TM1637_isBcdOfDisplayHead(dispContext->hour.ge, &bcdContext->hour.ge, &isFirstHaveData);
        TM1637_isBcdOfDisplayHead(dispContext->min.shi, &bcdContext->min.shi, &isFirstHaveData);
        TM1637_isBcdOfDisplayHead(dispContext->min.ge, &bcdContext->min.ge, &isFirstHaveData);
    }
}
/*
 控制显示，配置好数据结构体之后，直接调用该函数就可以显示
  */
static void TM1637_CMDDisplay(DispContext *dispContext, TM1637_DISPLAY_TYPE type)
{
    static INT8U isDisplayDot = 0;
    DispContext bcdContext;

    TM1637_GetBcdOfDisplay(dispContext, &bcdContext, type, isDisplayDot);
    isDisplayDot = !isDisplayDot;

    TM1637_WriteCommand(TM1637_DISPLAY_MODE_ADDR_MANUAL);
    TM1637_WriteData(TM1637_DISPLAY_ADDR_0, bcdContext.hour.shi);
    TM1637_WriteData(TM1637_DISPLAY_ADDR_1, bcdContext.hour.ge);
    TM1637_WriteData(TM1637_DISPLAY_ADDR_2, bcdContext.min.shi);
    TM1637_WriteData(TM1637_DISPLAY_ADDR_3, bcdContext.min.ge);
    // TIM1637_ChangePluseWidth();
    TM1637_WriteCommand(g_TM1637PluseWidth);
    // TM1637_DEBUG(("TM1637 Update time over"));
}
void TM1637_DisplayVal(TM1637_DISPLAY_TYPE type, INT32U val, INT32U dlyMs)
{
    g_dispPara.context.hour.shi = val / 1000;
    if (g_dispPara.context.hour.shi >= 10) {
        g_dispPara.context.hour.shi = TIM1637_ENCODE_OF_ERROR;
    }
    g_dispPara.context.hour.ge = val / 100 % 10;
    g_dispPara.context.min.shi = val / 10 % 10;
    g_dispPara.context.min.ge = val % 10;
    g_dispPara.lastTimeRefreshCycle = (dlyMs / TIM1637_REFRESH_CYCLE) + 1;
    g_dispPara.type = type;
    TM1637_CMDDisplay(&g_dispPara.context, g_dispPara.type);
}
/// @brief 显示时间的时候，可以临时显示其他内容，显示完之后，再显示时间
/// @param  
static void TM1637_DisplayTime(void)
{
    static bool isDisplayMinSec = true; // true:显示分秒，false:显示时分
    if (g_dispPara.lastTimeRefreshCycle > 0) {
        g_dispPara.lastTimeRefreshCycle--;
        if (g_dispPara.lastTimeRefreshCycle == 0) {
            g_dispPara.type = DISPLAY_TYPE_TIME;
        }
    }
    if (g_dispPara.type == DISPLAY_TYPE_TIME) {
        RTC_TimeTypeDef systime;
        RTC_GetDate(&systime, RTC_FORMAT_BIN);
        if (isDisplayMinSec) {
            g_dispPara.context.hour.shi = systime.Minutes / 10;
            g_dispPara.context.hour.ge = systime.Minutes % 10;
            g_dispPara.context.min.shi = systime.Seconds / 10;
            g_dispPara.context.min.ge = systime.Seconds % 10;
        } else {
            g_dispPara.context.hour.shi = systime.Hours / 10;
            g_dispPara.context.hour.ge = systime.Hours % 10;
            g_dispPara.context.min.shi = systime.Minutes / 10;
            g_dispPara.context.min.ge = systime.Minutes % 10;
        }
        isDisplayMinSec = !isDisplayMinSec;
    }
    TM1637_CMDDisplay(&g_dispPara.context, g_dispPara.type);
}

static void TM1637_TaskHandler(void *pArg)
{
    while (1)
    {
        vTaskDelay(TIM1637_REFRESH_CYCLE);
        TM1637_DisplayTime();
    }
}

__attribute__((unused)) static void TM1637_Init(void)
{
    TM1637_GPIOInit();
    xTaskCreate(TM1637_TaskHandler, "TM1637 DisplayTime", 128 * 2, NULL, 10, NULL);
}

AppInitCall(TM1637_Init);
