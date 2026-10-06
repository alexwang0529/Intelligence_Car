#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include <stdint.h>

void Bluetooth_Init(uint32_t baud);
void Bluetooth_SendByte(uint8_t data);
void Bluetooth_SendString(const char *str);
uint8_t Bluetooth_TryRead(uint8_t *data);

#endif
