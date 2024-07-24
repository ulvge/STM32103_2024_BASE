#ifndef __I2CFORWARD_H
#define	__I2CFORWARD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "semphr.h"

 void Task_I2cForWard(void *param);
#ifdef __cplusplus
}
#endif

#endif /* __I2CFORWARD_H */

