#include "stm32f10x.h"
#include "Bluetooth.h"

void Bluetooth_Init(uint32_t baud)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_9;       /* PA9: STM32 TX */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_10;      /* PA10: STM32 RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = baud;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

void Bluetooth_SendByte(uint8_t data)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {}
    USART_SendData(USART1, data);
}

void Bluetooth_SendString(const char *str)
{
    while (*str != '\0')
        Bluetooth_SendByte((uint8_t)*str++);
}

uint8_t Bluetooth_TryRead(uint8_t *data)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        return 0;

    *data = (uint8_t)USART_ReceiveData(USART1);
    return 1;
}
