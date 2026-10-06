#include "DirectionHold.h"
#include "Heading.h"
#include "Motor.h"

static int16_t basePwm = 35;
static uint8_t running = 0;
static uint8_t turningBack = 0;

#define HEADING_DEADBAND_DEG  5.0f

static float GetHeadingError(void)
{
    float error = -Heading_GetDeg(); /* 目标朝向为启动时的 0° */

    /* 取最短转向方向 */
    while (error > 180.0f)  error -= 360.0f;
    while (error < -180.0f) error += 360.0f;

    return error;
}

void DirectionHold_Start(int16_t forwardPwm)
{
    if (forwardPwm < 20) forwardPwm = 20;
    if (forwardPwm > 60) forwardPwm = 60;

    basePwm = forwardPwm;
    turningBack = 0;
    Heading_Zero();       /* 以此刻车头朝向为目标 */
    running = 1;
}

void DirectionHold_Update(int16_t gyroZ)
{
    float error;
    float absError;
    int16_t correction;

    Heading_Update(gyroZ);

    if (!running)
        return;

    error = GetHeadingError();
    absError = (error >= 0.0f) ? error : -error;

    /* 偏差较大时，先原地转回目标方向 */
    if (turningBack)
    {
        if (absError <= 5.0f)
            turningBack = 0;
    }
    else if (absError >= 25.0f)
    {
        turningBack = 1;
    }

    if (turningBack)
    {
        if (error > 0.0f)
            Motor_SetSides(-25, 25);  /* 逆时针 */
        else
            Motor_SetSides(25, -25);  /* 顺时针 */

        return;
    }

    /* 小于等于5度时不修正，避免陀螺仪噪声引起左右抖动 */
    if (absError <= HEADING_DEADBAND_DEG)
    {
        Motor_SetSides(basePwm, basePwm);
        return;
    }

    /* 小偏差：行驶中调整左右轮速度 */
    correction = (int16_t)(error * 1.0f);

    if (correction > 20)  correction = 20;
    if (correction < -20) correction = -20;

    Motor_SetSides(basePwm - correction,
                   basePwm + correction);
}

void DirectionHold_Stop(void)
{
    running = 0;
    turningBack = 0;
    Motor_StopAll();
}
