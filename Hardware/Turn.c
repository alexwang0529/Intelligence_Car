/*
*转向指定角度
*/

#include "stm32f10x.h"
#include "Delay.h"
#include "Motor.h"
#include "MPU6050.h"
#include "Heading.h"
#include "Turn.h"

#define DWT_CYCCNT_REG (*(volatile uint32_t *)0xE0001004UL)

Turn_Result Turn_Relative(float targetDeg)
{
    uint32_t startCycles;
    uint32_t elapsedCycles;
    int16_t ax, ay, az, gx, gy, gz;
    int16_t pwm;
    float angle;
    float remaining;

	
    if (targetDeg < -360.0f || targetDeg > 360.0f)
        return TURN_INVALID_ANGLE;

    if (targetDeg == 0.0f)
        return TURN_OK;

    Heading_Zero();
    startCycles = DWT_CYCCNT_REG;

    while (1)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        Heading_Update(gz);
        angle = Heading_GetDeg();

        //沿目标转动方向，还差多少度
        remaining = (targetDeg > 0.0f)
                  ? targetDeg - angle
                  : angle - targetDeg;

		//避免抖动
        if (remaining <= 5.0f)
        {
            Motor_StopAll();
            return TURN_OK;
        }

        elapsedCycles = DWT_CYCCNT_REG - startCycles;

        if (elapsedCycles > SystemCoreClock * 15UL)
        {
            Motor_StopAll();
            return TURN_TIMEOUT;
        }

        /* 轮子转了但车身没转：避免持续堵转 */
        if (elapsedCycles > SystemCoreClock &&
            angle > -2.0f && angle < 2.0f)
        {
            Motor_StopAll();
            return TURN_NO_MOTION;
        }

        if ((targetDeg > 0.0f && angle < -10.0f) ||
            (targetDeg < 0.0f && angle > 10.0f))
        {
            Motor_StopAll();
            return TURN_WRONG_DIRECTION;
        }

        //接近目标时降低 PWM
		//减速梯度不够可能会导致转向不精准
        if (remaining > 45.0f)
            pwm = 40;
        else if (remaining > 25.0f)
            pwm = 30;
        else if (remaining > 10.0f)
			pwm = 20;
		else
			pwm = 10;
		

        if (targetDeg > 0.0f)       /* 逆时针 */
            Motor_SetSides(-pwm, pwm);
        else                        /* 顺时针 */
            Motor_SetSides(pwm, -pwm);

        Delay_ms(10);
    }
}
