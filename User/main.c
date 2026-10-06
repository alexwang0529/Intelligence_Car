#include "stm32f10x.h"
#include "Motor.h"
#include "Encoder.h"
#include "OLED.h"
#include "Key.h"
#include "MPU6050.h"
#include "Heading.h"
#include "Ultrasonic.h"
#include "Menu.h"
#include "Mode.h"
#include "Delay.h"
#include "LineSensor.h"


int main(void)
{
    SystemCoreClockUpdate();

    Motor_Init();         
    Motor_StopAll();
    Key_Init();
    Encoder_Init();
    OLED_Init();
    MPU6050_Init();
    Heading_Init();        
    Ultrasonic_Init();
	LineSensor_Init();

    while (1)
    {
        Mode_Run(Menu_Select());
    }
}
