#ifndef __DEV_KEY_H
#define __DEV_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    UINT32 isLastPressed : 1; // May be invalid
    UINT32 isPreesed : 1;     // valid Preesed
    UINT32 isReleased : 1;
    GPIO_Idex pin;
    UINT32 preesedStartTick;
} Key_ScanST;

UINT32 KeyPressedDurationMs(Key_ScanST *key);

#ifdef __cplusplus
}
#endif

#endif /* __API_SUB_DEVICES_H */

