
#ifndef  _TM1637_DRV_H
#define  _TM1637_DRV_H

#include "stm32f1xx.h"
#include "Types.h"	




//引脚的时钟使能函数的宏定义
//DIO和CLK相关引脚的宏定义，用户只需要修改相关宏即可
#define    TM_DIO_PORT    	GPIOB		                 
#define    TM_DIO_CLK 	    __HAL_RCC_GPIOB_CLK_ENABLE		
#define    TM_DIO_PIN		GPIO_PIN_15			        

#define    TM_CLK_PORT    	GPIOB			              
#define    TM_CLK_CLK 	    __HAL_RCC_GPIOB_CLK_ENABLE		
#define    TM_CLK_PIN		GPIO_PIN_13		

//时钟线和数据线高低电平的宏定义，不需修改
#define      DIO_1         HAL_GPIO_WritePin(TM_DIO_PORT, TM_DIO_PIN, GPIO_PIN_SET);
#define      DIO_0         HAL_GPIO_WritePin(TM_DIO_PORT, TM_DIO_PIN, GPIO_PIN_RESET);
#define      CLK_1         HAL_GPIO_WritePin(TM_CLK_PORT, TM_CLK_PIN, GPIO_PIN_SET);
#define      CLK_0         HAL_GPIO_WritePin(TM_CLK_PORT, TM_CLK_PIN, GPIO_PIN_RESET);

typedef struct {
	char shi;
	char ge;
}DispHour, DispMin;

typedef struct {
	DispHour hour;
	DispMin min;
}DispContext;

typedef enum {
	DISPLAY_TYPE_TIME,
	DISPLAY_TYPE_TEMPERATURE,
	DISPLAY_TYPE_FREQUENCY,
}TM1637_DISPLAY_TYPE;

typedef struct {
	TM1637_DISPLAY_TYPE type;
	INT32U lastTimeRefreshCycle;
	DispContext context;
}DispPara;

//相关函数声明  

void TM1637_DisplayVal(TM1637_DISPLAY_TYPE type, INT32U val, INT32U dlyMs);
#endif

