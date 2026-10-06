#ifndef __LINE_SENSOR_H
#define __LINE_SENSOR_H

#include <stdint.h>

void LineSensor_Init(void);
uint8_t LineSensor_ReadRaw(void);
int8_t LineSensor_GetError(uint8_t value);

#endif
