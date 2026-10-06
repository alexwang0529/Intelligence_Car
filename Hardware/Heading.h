#ifndef __HEADING_H
#define __HEADING_H

#include <stdint.h>

void Heading_Init(void);
void Heading_CalibrateBias(void);
void Heading_Update(int16_t gyroZ);
void Heading_Zero(void);
float Heading_GetDeg(void);

#endif
