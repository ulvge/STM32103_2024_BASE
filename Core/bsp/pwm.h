/****************************************Copyright (c)****************************************************
**                            Shenzhen HONGMEN electronics Co.,LTD.
**
**                                 http://www.hong-men.com
**
**--------------File Info---------------------------------------------------------------------------------
** File Name          : PWM.H
** Last modified Date : 2011-06-03
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
#ifndef _PWM_H
#define _PWM_H
#include "stm32f1xx.h"
#include "Types.h"	
   
#define PWM_GPIO    GPIOA
#define PWM_PIN     GPIO_PIN_8

void PWM_DutyChange(BOOLEAN isAdd);
void TIM1_UP_IRQHandler(void);

int PWMStartHandler(int argc, char *argv[], int index);
int PWMStopHandler(int argc, char *argv[], int index);

#endif  



