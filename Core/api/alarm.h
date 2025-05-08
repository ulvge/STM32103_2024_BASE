#ifndef __ALARM_H
#define	__ALARM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "semphr.h"

extern SemaphoreHandle_t semaphoreStopAlarm;
void Alarm_Task(void *param);

#ifdef __cplusplus
 }
#endif

#endif /* __ALARM_H */

