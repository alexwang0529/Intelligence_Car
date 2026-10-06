#ifndef __OBJECT_COUNTER_H
#define __OBJECT_COUNTER_H

#include <stdint.h>

#define OBJECT_MAX_RECORDS  32

void ObjectCounter_Reset(void);
void ObjectCounter_Update(uint16_t sensorCm);
uint16_t ObjectCounter_GetCount(void);
uint16_t ObjectCounter_GetDistance(uint16_t index);

#endif
