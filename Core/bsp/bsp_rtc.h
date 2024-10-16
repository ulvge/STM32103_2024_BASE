#ifndef __BSP_RTC_H
#define	__BSP_RTC_H

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx.h"

void RTC_Init(void);
void RTC_GetDate(RTC_TimeTypeDef *time, uint32_t Format);
#ifdef __cplusplus
}
#endif

#endif /* __API_UTC_H */

