#ifndef __DEV_KEY_H
#define __DEV_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef void (*KeyReleasedHandler)(void);
typedef void (*KeyPressedHandler)(void);

typedef struct {
    UINT32 isLastPressed : 1; // May be invalid
    UINT32 isPreesed : 1;     // valid Preesed
    UINT32 isReleased : 1;
    GPIO_Idex pin;
    UINT32 preesedStartTick;
    KeyReleasedHandler onReleased;
    KeyPressedHandler onPressed;
} Key_ScanST;

UINT32 KeyPressedDurationMs(Key_ScanST *key);
bool key_handlerRegister(GPIO_Idex idx, KeyPressedHandler onPressed, KeyReleasedHandler onReleased);
bool key_handlerUnRegister(GPIO_Idex idx);
#ifdef __cplusplus
}
#endif

#endif /* __API_SUB_DEVICES_H */

