/*
*通过MPU6050确认小车当前角度
*/

#include "stm32f10x.h"
#include "Delay.h"
#include "MPU6050.h"
#include "Heading.h"

/* 旧版 CMSIS 头文件未定义 DWT，直接使用 Cortex-M3 寄存器地址 */
#define DWT_CTRL_REG    (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT_REG  (*(volatile uint32_t *)0xE0001004UL)

static float gyroZBias = 0.0f;    // 陀螺仪 Z 轴静止时的偏差
static float angleDeg = 0.0f;     // 当前累计的转动角度，单位：度
static uint32_t lastCycles = 0;   // 上次更新角度时的 DWT 计数值

void Heading_Init(void)
{
    uint16_t i;
    int32_t sum = 0;
    int16_t ax, ay, az, gx, gy, gz;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; //开启 Cortex-M3 的调试与追踪功能，让后面能够使用 DWT
    DWT_CYCCNT_REG = 0;
    DWT_CTRL_REG |= 1UL;
	
	Delay_ms(1000);

    /* 启动时小车必须静止 */
    for (i = 0; i < 200; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        sum += gz;
        Delay_ms(5);
    }

    gyroZBias = (float)sum / 200.0f;  //计算实际偏差
    angleDeg = 0.0f;
    lastCycles = DWT_CYCCNT_REG;
}

/* 小车静止时重新测量陀螺仪 Z 轴零偏 */
void Heading_CalibrateBias(void)
{
    uint16_t i;
    int32_t sum = 0;
    int16_t ax, ay, az, gx, gy, gz;

    for (i = 0; i < 100; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        sum += gz;
        Delay_ms(5);
    }

    gyroZBias = (float)sum / 100.0f;
    angleDeg = 0.0f;
    lastCycles = DWT_CYCCNT_REG;
}

void Heading_Update(int16_t gyroZ)
{
    uint32_t now = DWT_CYCCNT_REG;
    uint32_t elapsedCycles = now - lastCycles;
    float dt; //时间间隔
    float rateDegPerSecond;

    lastCycles = now;
    dt = (float)elapsedCycles / (float)SystemCoreClock;

    /* 调试器长时间暂停后，不把暂停时间计入角度 */
    if (dt > 0.5f)
        return;

    /* 当前 GYRO_CONFIG=0x18，即 ±2000°/s、16.4 计数/(°/s) */
	//先减去静止时测得的偏差，再把陀螺仪原始值换算成角速度
    rateDegPerSecond = ((float)gyroZ - gyroZBias) / 16.4f;  

	if (rateDegPerSecond > -0.5f && rateDegPerSecond < 0.5f)
		rateDegPerSecond = 0.0f;

	//“角速度 × 时间”得到这段时间转过的角度
	angleDeg += rateDegPerSecond * dt; 
}

//当前朝向置0
void Heading_Zero(void)
{
    angleDeg = 0.0f;
    lastCycles = DWT_CYCCNT_REG;
}

float Heading_GetDeg(void)
{
    return angleDeg;
}
