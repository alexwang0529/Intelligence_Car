#ifndef __ENCODER_H
#define __ENCODER_H



void Encoder_Init(void);
int32_t Encoder_GetLeftCount(void);
int32_t Encoder_GetRightCount(void);
void Encoder_IRQHandler(void);

#endif
