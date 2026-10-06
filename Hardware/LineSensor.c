#include "stm32f10x.h"

void LineSensor_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |
                    GPIO_Pin_10 | GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);
}

uint8_t LineSensor_ReadRaw(void)
{
    uint8_t value = 0;

	//把结果保存到 value 的第 3 位
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0))  value |= 0x08; // X1
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1))  value |= 0x04; // X2
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10)) value |= 0x02; // X3
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) value |= 0x01; // X4

    return value;
}

int8_t LineSensor_GetError(uint8_t value)
{
    int8_t error = 0;
    uint8_t count = 0;

    /*
     * 实际排列：X2、X1、X3、X4
     * 黑线电平：0
     * 左侧为负，右侧为正
     */

    if ((value & 0x04) == 0)   // X2，最左
    {
        error -= 3;
        count++;
    }

    if ((value & 0x08) == 0)   // X1，左中
    {
        error -= 1;
        count++;
    }

    if ((value & 0x02) == 0)   // X3，右中
    {
        error += 1;
        count++;
    }

    if ((value & 0x01) == 0)   // X4，最右
    {
        error += 3;
        count++;
    }

    if (count == 0)
        return 127;             // 全部白底，暂时表示丢线

    return error;
}
