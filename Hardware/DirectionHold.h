#ifndef __DIRECTION_HOLD_H
#define __DIRECTION_HOLD_H

#include <stdint.h>

void DirectionHold_Start(int16_t forwardPwm);
void DirectionHold_Update(int16_t gyroZ);
void DirectionHold_Stop(void);

#endif
