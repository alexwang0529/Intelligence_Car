/*
*电机编码器
*/

#include "stm32f10x.h"
#include "Encoder.h"


/* PB12/PB13 测左后轮；PB14/PB15 测右前轮 */
/* 保证编码器正转为正，反转为负 */
#define LEFT_ENCODER_SIGN   1
#define RIGHT_ENCODER_SIGN  -1

static volatile int32_t left_count = 0;
static volatile int32_t right_count = 0;

void Encoder_Init(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,  
        ENABLE);

    /* PB12/PB13：左后 A/B；PB14/PB15：右前 A/B */
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 |
                    GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //浮空输入
    gpio.GPIO_Speed = GPIO_Speed_2MHz;		  
    GPIO_Init(GPIOB, &gpio);

    /* 只让两台编码器的 A 相触发中断 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

    EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line14);  //清除中断标志位

    exti.EXTI_Line = EXTI_Line12 | EXTI_Line14;			
    exti.EXTI_Mode = EXTI_Mode_Interrupt; //中断模式
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    /* EXTI12 和 EXTI14 共用 EXTI15_10 中断 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    nvic.NVIC_IRQChannel = EXTI15_10_IRQn; 		//中断入口
    nvic.NVIC_IRQChannelPreemptionPriority = 1; //中断优先级
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

//中断计数（左右）
void Encoder_IRQHandler(void)
{
    uint8_t a;
    uint8_t b;

    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
        b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);

        left_count += (a != b) ?
            LEFT_ENCODER_SIGN : -LEFT_ENCODER_SIGN;

        EXTI_ClearITPendingBit(EXTI_Line12);
    }

    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
        b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);

        right_count += (a != b) ?
            RIGHT_ENCODER_SIGN : -RIGHT_ENCODER_SIGN;

        EXTI_ClearITPendingBit(EXTI_Line14);
    }
}

int32_t Encoder_GetLeftCount(void)
{
    return left_count;
}

int32_t Encoder_GetRightCount(void)
{
    return right_count;
}
