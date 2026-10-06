/*
*超声波模块
*/

#include "stm32f10x.h"
#include "Delay.h"
#include "Ultrasonic.h"

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timer;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_4;             /* TRIG */
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);

    gpio.GPIO_Pin = GPIO_Pin_5;             /* ECHO，已外部分压 */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* 当前工程系统时钟 72 MHz；TIM4 每 1 us 计数一次 */
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = 71;
    timer.TIM_Period = 65535;
    TIM_TimeBaseInit(TIM4, &timer);
    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

uint16_t Ultrasonic_ReadCm(void)
{
    uint16_t pulse_us;

    /* 等上一次回波结束 */
    TIM_SetCounter(TIM4, 0);
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) != Bit_RESET)
    {
        if (TIM_GetCounter(TIM4) >= 30000)  //太久没有回拨
            return ULTRASONIC_NO_ECHO;
    }

    /* 发送至少 10 us 的触发脉冲 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    Delay_us(2);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);     //发送（高电平）
    Delay_us(10);
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);

    /* 等待 ECHO 上升沿 */
    TIM_SetCounter(TIM4, 0);   //开始计数，计数置0
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == Bit_RESET) //无回波
    {
        if (TIM_GetCounter(TIM4) >= 30000)
            return ULTRASONIC_NO_ECHO;
    }

    /* 测量 ECHO 高电平宽度 */
    TIM_SetCounter(TIM4, 0);
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) != Bit_RESET)
    {
        if (TIM_GetCounter(TIM4) >= 30000)
            return ULTRASONIC_NO_ECHO;
    }

    pulse_us = TIM_GetCounter(TIM4);

    /* 往返声程约 58 us 对应目标距离 1 cm */
    return (pulse_us + 29) / 58;  //四舍五入
}
