/****************************************Copyright (c)****************************************************
**                            Shenzhen HONGMEN electronics Co.,LTD.
**
**                                 http://www.hong-men.com
**
**--------------File Info---------------------------------------------------------------------------------
** File Name          : PWM.C
** Last modified Date : 2011-06-02
** Last Version       : V1.00
** Descriptions       : TIMER File
**
**--------------------------------------------------------------------------------------------------------
** Modified by        :
** Modified date      :
** Version            :
** Descriptions       :
**
*********************************************************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include "stm32f1xx.h"
#include "debug_print.h"
#include	"pwm.h"	
#include	"main.h"
#include	"initcall.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_gpio.h"

/****************************************************************
 * 函数名：void GPIO_Config(void) 
 * 描述  ：配置复用输出PWM时用到的I/O 
 * 输入  ：无 
 * 输出  ：无 
 * 调用  ：main()调用 
● TIM1_CH1 pin (PA8)   pwm
● TIM1_CH1N pin (PB13)
● TIM1_CH2 pin (PA9)
● TIM1_CH2N pin (PB14)
● TIM1_CH3 pin (PA10)
● TIM1_CH3N pin (PB15)
 ***************************************************************/
#define PWM_TIM                     TIM1
#define PWM_CHANNEL                 TIM_CHANNEL_1

#define PWM_PERCNET_DEFAULT         50	
#define PWM_RESOLUTION_DEFAULT      5	//step

//32M系统时钟，不分频：32M, 最高频率32M
//32M系统时钟，32分频：1M, 最高频率1M.
//TIM_Period = 1000，即1ms; 则频率为1000HZ
//TIM_Period = 500，即0.5ms; 则频率为2000HZ

//频率为1000HZ,即周期1ms，  TIM_Period = 1000
//频率为2000HZ 即周期0.5ms，TIM_Period = 500
#define PWM_WORK_FREQUENCY  1500
#define PWM_INT_REPETNUMS  150  //多少次之后，才上报一次中断，避免频繁中断

#define PWM_PRE_SCALE_DIV_1M   (SYS_CORE_CLOCK_X_MHZ)
#define PWM_PRE_SCALE_DIV_1K  (SYS_CORE_CLOCK_X_KHZ)
typedef struct {
	INT32U	freMax;		           //当前预分频的最高支持的PWM频率
	INT32U	preScale;              //预分频的值
	//设定分割频率，根据滤波系数决定的采样频率和N事件有利于滤除高频干扰信号
	INT16U	clkDiv;               //和PWM死区信号有关 https://blog.csdn.net/yuyan7045/article/details/121289037?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.nonecase&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.nonecase
    INT8U   polarity;
	INT8U	repetNums;              //重复上报的值，宽度8bit
}FrequecnyPreScale_ST;
static const FrequecnyPreScale_ST g_tablePreScale[] = {
	{100,			PWM_PRE_SCALE_DIV_1K, TIM_CLOCKDIVISION_DIV1, TIM_OCPOLARITY_LOW, PWM_INT_REPETNUMS},
	{1000 * 1000,	PWM_PRE_SCALE_DIV_1M, TIM_CLOCKDIVISION_DIV1, TIM_OCPOLARITY_LOW, PWM_INT_REPETNUMS},
};

typedef enum {
	PWM_DispMode_CntSlef = (INT8U)0,	//自增
	PWM_DispMode_Table,			//查表
	PWM_DispMode_Remote,			//远程控制
	PWM_DispMode_MAX,
}PWM_DispMode_ST;

//通过shell，来选择LED模式,
typedef struct {
    const FrequecnyPreScale_ST *pBestWorkMode;
	INT32U	workFreq;
	INT16U	Perio;      //周期
	INT16U	Resolution; //分辨率 按键加/减 时的stepVal的值

	INT16U	DispRGBMode;
	INT16U	DispSpeedMode;
	INT16U	DispTbIndex;//TBColor中的index
} PWM_CFG;
PWM_CFG  PwmCfg;

TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef sConfig;
static void PWM_CfgRGBMode(INT8U mode);
static const FrequecnyPreScale_ST *PWM_getBestWorkParaByFreq(INT32U freq)
{
	INT32U num = sizeof(g_tablePreScale) / sizeof(g_tablePreScale[0]);
	INT32U idx = 0;
	for (idx = 0; idx < num - 1; idx++) {
		if (freq <= g_tablePreScale[idx].freMax) {
			break;
		}
	}
    return &g_tablePreScale[idx];
}
static INT16U PWM_getPWMDefaultConfig(PWM_CFG* config)
{
	config->Resolution = PWM_RESOLUTION_DEFAULT;
	config->DispSpeedMode = 1;
	config->DispTbIndex = 0;
	//PWM_CfgRGBMode(PWM_DispMode_CntSlef);
	PWM_CfgRGBMode(PWM_DispMode_Remote);

	config->pBestWorkMode = PWM_getBestWorkParaByFreq(PWM_WORK_FREQUENCY);
	config->workFreq = SYS_CORE_CLOCK / config->pBestWorkMode->preScale / ((config->pBestWorkMode->clkDiv>>8) + 1);

	INT32U period = config->workFreq / PWM_WORK_FREQUENCY;
	if (period > 0xFFFF) {
		return FALSE;
	}
	config->Perio = period;
	return TRUE;
}

void PWM_ReqRGBMode(void)
{   
	LOG_D("->: PWM CfgMode  = 0x%o\n", PwmCfg.DispRGBMode);
}
static void PWM_CfgRGBMode(INT8U mode)
{   
    if(mode >= PWM_DispMode_MAX){
		LOG_D("->err: PWM CfgMode  = 0x%o\n", mode);
		return;
	}
	PwmCfg.DispRGBMode= mode;
	LOG_D("->: PWM CfgMode  = 0x%o\n", mode);
}
void PWM_SetDuty(TIM_TypeDef* TIMx,INT16U pin,INT16U pluseNum)
{
    sConfig.OCMode       = TIM_OCMODE_PWM1;
    sConfig.OCPolarity   = PwmCfg.pBestWorkMode->polarity;
    sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    sConfig.Pulse = pluseNum;
    switch(pin){
        case PWM_PIN:
            if (pluseNum){
                if (HAL_TIM_PWM_Stop(&TimHandle, PWM_CHANNEL) != HAL_OK)
                {
                }  
                GPIO_ReInitGPIO(GPIO_PWM_AF);
                if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, PWM_CHANNEL) != HAL_OK)
                {
                }
                if (HAL_TIM_PWM_Start(&TimHandle, PWM_CHANNEL) != HAL_OK)
                {
                }
            }else{
                HAL_TIM_PWM_Stop(&TimHandle, PWM_CHANNEL);
                GPIO_ReInitGPIO(GPIO_PWM_PP);
            }
            break;
        default:
            return;
    }
    return;
}
/*
*dutyPercent:值[0,100]
PwmCfg.Resolution  :分辨率 100，从0~99，100个阶梯
*/
static INT16U  PWM_calcDutyByPercent(INT8U dutyPercent)
{
	INT16U pluseNum;
	pluseNum = PwmCfg.Perio * dutyPercent / 100;
	return pluseNum;
}          
static INT16U dutyPercent = PWM_PERCNET_DEFAULT;
void PWM_DutyChange(BOOLEAN isAdd)
{              
	if (isAdd) {
		if (dutyPercent + PwmCfg.Resolution < 100) {
			dutyPercent += PwmCfg.Resolution;
		}else{
            dutyPercent = 100;
        }
	} else {
		if (dutyPercent >= PwmCfg.Resolution) {
			dutyPercent -= PwmCfg.Resolution;
		} else{
            dutyPercent = 0;
        }
	} 
	LOG_D("->: PWM DutyChange [%d]\n", dutyPercent); 
    PWM_SetDuty(PWM_TIM, PWM_PIN, PWM_calcDutyByPercent(dutyPercent));
}
static void PWM_TIM1_Config(PWM_CFG* pwmConfig)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    TimHandle.Instance = PWM_TIM;
    TimHandle.Init.Prescaler         = pwmConfig->pBestWorkMode->preScale - 1;
    TimHandle.Init.Period            = pwmConfig->Perio - 1;
    TimHandle.Init.ClockDivision     = pwmConfig->pBestWorkMode->clkDiv;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = pwmConfig->pBestWorkMode->repetNums; //每次向上溢出都产生更新事件;向上溢出,重复x次后，才上报一次中断
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

	// //设置PWM_TIM的PWM输出为使能
	// TIM_CtrlPWMOutputs(PWM_TIM, ENABLE);
	
	LOG_D("->: PWM_Duty percent default [%d]\n", PWM_PERCNET_DEFAULT);
}

__attribute__((unused)) static void PWN_ITConfig(void)
{
    /*##-2- Configure the NVIC for TIMx #########################################*/
	// //使能TIM1中断源 
    __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
    HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 5, 1);
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

}
/// <summary>
/// 根据频率，当中断次数==频率时，打印输出，确认时间，是否刚好是1s
/// </summary>
/// <param name=""></param>
void TIM1_UP_IRQHandler(void)
{
    // PWM_INT_REPETNUMS 上报一次中断，interruptCount次中断后，才打印一次
	#define interruptCount (PWM_WORK_FREQUENCY / (PWM_INT_REPETNUMS))
	static INT32U count;
	if (__HAL_TIM_GET_IT_SOURCE(&TimHandle, TIM_IT_UPDATE) != RESET) {//检查指定的TIM中断发生与否:TIM 中断源 
		__HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE);//清除TIMx的中断待处理位:TIM 中断源 
	}
	count++;
	if (count >= interruptCount) {
		count = 0;
        //LOG_D("->: PWM work ...frequency[%d] \n", PWM_WORK_FREQUENCY);
	}
}
static int pwmShellDuty(int argc, char *argv[])
{
    if (argc !=2) {
        LOG_RAW("pwm 30, set duty = 30\n");
        return 0;
    }
    
    INT8U dutyPercent = strtol(argv[1], NULL, 0);
    if (dutyPercent > 100) {
        dutyPercent = 100;
    }
    PWM_SetDuty(PWM_TIM, PWM_PIN, PWM_calcDutyByPercent(dutyPercent));  
    return 0;
}
static void PWM_Init(void)
{
    if (PWM_getPWMDefaultConfig(&PwmCfg) == FALSE) {
		LOG_D("->: PWM freqency of config is error\n");
		while (1);
	}

	PWM_TIM1_Config(&PwmCfg);
	PWM_SetDuty(PWM_TIM, PWM_PIN, PWM_calcDutyByPercent(PWM_PERCNET_DEFAULT));
	PWN_ITConfig();
}

CoreInitCall(PWM_Init);

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, pwm, pwmShellDuty, set pwm duty);

