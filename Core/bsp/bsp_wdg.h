#ifndef _BSP_WDG_H
#define _BSP_WDG_H

#include <stdint.h>
#include <stdbool.h>

void WatchDog_init(void);
void WatchDog_feed(void);

uint32_t ResetCause_Get(void);
void ResetCause_Printf(void);

#endif
