#ifndef __TURN_H
#define __TURN_H

typedef enum
{
    TURN_OK = 0,
    TURN_NO_MOTION,
    TURN_WRONG_DIRECTION,
    TURN_TIMEOUT,
    TURN_INVALID_ANGLE
} Turn_Result;

Turn_Result Turn_Relative(float targetDeg);

#endif
