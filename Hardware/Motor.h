#ifndef __MOTOR_H
#define __MOTOR_H



void Motor_Init(void);
void Motor_Set(uint8_t motor, int16_t speed); /* 电机编号 1~4；速度 -100~100 */
void Motor_StopAll(void);
void Motor_SetSides(int16_t left, int16_t right);

#endif
