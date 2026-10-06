#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include <stdint.h>

#define ULTRASONIC_NO_ECHO  0xFFFF

void Ultrasonic_Init(void);
uint16_t Ultrasonic_ReadCm(void);

#endif
