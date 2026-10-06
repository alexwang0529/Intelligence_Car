#include "stm32f10x.h"
#include "Delay.h"
#include "Key.h"
#include "OLED.h"
#include "Motor.h"
#include "MPU6050.h"
#include "Heading.h"
#include "DirectionHold.h"
#include "Turn.h"
#include "Ultrasonic.h"
#include "Mode.h"
#include "Encoder.h"
#include "ObjectCounter.h"
#include "LineSensor.h"

#define WHEEL_COUNTS_PER_REV  728.0f       //电机编码器转一圈计数
#define WHEEL_DIAMETER_CM     6.5f		   //轮子直径6.5cm
#define PI_VALUE              3.1415926f   //PI值
#define DWT_CYCCNT_REG       (*(volatile uint32_t *)0xE0001004UL)   //计数器（可以实时读取）
#define LINE_GAP_CM           60.0f         //地图中的空白段长度


//按键控制
static uint8_t WaitKey(void)
{
    uint8_t key;

	
	//消抖
    do
    {
        key = Key_GetNum();
        if (key == 0)
            Delay_ms(20);
    } while (key == 0);

    return key;
}

//必须松开按键才执行

static void WaitKey2(void)
{
    while (WaitKey() != 2) {}
}


//1-自动纠偏
static void RunHold(void)
{
    int16_t ax, ay, az, gx, gy, gz;
    uint8_t displayCount = 0;

    OLED_Clear();
    OLED_ShowString(1, 1, "DIRECTION HOLD");
    OLED_ShowString(2, 1, "KEEP STILL");
    OLED_ShowString(3, 1, "CALIBRATING");

    Motor_StopAll();
    Heading_CalibrateBias();

    OLED_ShowString(2, 1, "RUNNING     ");
    OLED_ShowString(3, 1, "K2: STOP    ");
    OLED_ShowString(4, 1, "ANG:        ");

    DirectionHold_Start(35);

    while (1)
    {
        /* 按下 K2 就先停电机，再等按键松开 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            DirectionHold_Stop();

            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);

            return;
        }

        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        DirectionHold_Update(gz);

        displayCount++;
        if (displayCount >= 5)
        {
            displayCount = 0;
            OLED_ShowSignedNum(4, 5,
                               (int32_t)Heading_GetDeg(), 4);
        }

        Delay_ms(20);
    }
}

//2-指定角度
static void RunAngle(void)
{
    uint8_t key;
    int8_t direction = 1;   /* 1：逆时针；-1：顺时针 */
    uint16_t angle = 0;
    Turn_Result result;

    /* 第一步：选择方向 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "TURN DIRECTION");
        OLED_ShowString(2, 1,
                direction == 1 ? "LEFT" : "RIGHT");
        OLED_ShowString(3, 1, "K1: CHANGE");
        OLED_ShowString(4, 1, "K2: CONFIRM");

        key = WaitKey();
        if (key == 1)
            direction = -direction;
        else if (key == 2)
            break;
    }

    /* 第二步：选择角度，0～360 度，每次增加 30 度 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "TURN ANGLE");
        OLED_ShowNum(2, 1, angle, 3);
        OLED_ShowString(2, 4, " DEG");
        OLED_ShowString(3, 1, "K1: +30 DEG");
        OLED_ShowString(4, 1, "K2: START");

        key = WaitKey();
        if (key == 1)
        {
            angle += 30;
            if (angle > 360)
                angle = 0;
        }
        else if (key == 2)
        {
            break;
        }
    }

    OLED_Clear();
    OLED_ShowString(1, 1, "TURNING...");

    result = Turn_Relative((float)direction * (float)angle);  //转动指定角度

    OLED_Clear();
    if (result == TURN_OK)
        OLED_ShowString(2, 1, "TURN OK");
    else
        OLED_ShowString(2, 1, "TURN FAILED");

    OLED_ShowString(4, 1, "K2: BACK");
    WaitKey2();
}

//3-里程计
static void RunOdometry(void)
{
    uint16_t targetCm = 50;
    uint16_t actualCm;
    uint8_t key;
    uint8_t refresh = 0;
    uint8_t success = 0;
    uint16_t noProgress = 0;
    int16_t ax, ay, az, gx, gy, gz;
    int32_t leftStart, rightStart;
    int32_t left, right, average;
    int32_t lastProgress = 0;
    int32_t targetCounts;
    uint32_t startCycles;
	float headingError;
	int16_t steering;

    /* K1 选择 50～100 cm；K2 开始 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "SET DISTANCE");
        OLED_ShowString(2, 1, "TARGET:     cm");
        OLED_ShowNum(2, 9, targetCm, 3);
        OLED_ShowString(3, 1, "K1: +10 cm");
        OLED_ShowString(4, 1, "K2: START");

        key = WaitKey();

        if (key == 1)
        {
            targetCm += 10;
            if (targetCm > 100)
                targetCm = 50;
        }
        else if (key == 2)
        {
            break;
        }
    }

    leftStart = Encoder_GetLeftCount();
    rightStart = Encoder_GetRightCount();

    targetCounts = (int32_t)(
        targetCm * WHEEL_COUNTS_PER_REV /
        (PI_VALUE * WHEEL_DIAMETER_CM) + 0.5f);

    OLED_Clear();
    OLED_ShowString(1, 1, "DRIVING");
    OLED_ShowString(2, 1, "GOAL:     cm");
    OLED_ShowNum(2, 7, targetCm, 3);
    OLED_ShowString(3, 1, "NOW:      cm");
    OLED_ShowString(4, 1, "K2: STOP");

    startCycles = DWT_CYCCNT_REG;  //起始时间

	Heading_Zero();
	OLED_ShowString(4, 1, "ANG:");

    while (1)
    {
        /* 行驶中按 K2 立即停车 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            Motor_StopAll();

            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);

            return;
        }

        left = Encoder_GetLeftCount() - leftStart;
        right = Encoder_GetRightCount() - rightStart;
        average = (left + right) / 2;                  //左右计数取平均

        if (average >= targetCounts) //达到目标计数
        {
            success = 1;             
            break;
        }

        /* 编码器不增长时停止，避免电机一直运行 */
        if (average > lastProgress + 2)					
        {
            lastProgress = average;
            noProgress = 0;
        }
        else
        {
            noProgress++;
            if (noProgress >= 75)
                break;
        }

        /* 最长运行 20 秒 */
        if ((uint32_t)(DWT_CYCCNT_REG - startCycles) >
            SystemCoreClock * 20UL)
            break;

        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
		Heading_Update(gz);

		headingError = -Heading_GetDeg();
		steering = (int16_t)(headingError * 0.6f);  //0.6f 表示每偏差 1°，约调整 0.6 个 PWM 单位

		if (steering > 8)  steering = 8;
		if (steering < -8) steering = -8;

/* 两侧始终向前，只做小幅方向修正 */
Motor_SetSides(35 - steering, 35 + steering);

        refresh++;
        if (refresh >= 5)
        {
            refresh = 0;
            actualCm = (average > 0)
                ? (uint16_t)(average * PI_VALUE *
                    WHEEL_DIAMETER_CM / WHEEL_COUNTS_PER_REV)
                : 0;

            OLED_ShowNum(3, 6, actualCm, 3);
			OLED_ShowSignedNum(4, 5, (int32_t)Heading_GetDeg(), 4);
        }

        Delay_ms(20);
    }

    Motor_StopAll();

    left = Encoder_GetLeftCount() - leftStart;
    right = Encoder_GetRightCount() - rightStart;
    average = (left + right) / 2;

    actualCm = (average > 0)
        ? (uint16_t)(average * PI_VALUE *
            WHEEL_DIAMETER_CM / WHEEL_COUNTS_PER_REV)
        : 0;

    OLED_Clear();
    OLED_ShowString(1, 1,
                    success ? "DISTANCE DONE" : "DISTANCE FAIL");
    OLED_ShowString(2, 1, "TRAVEL:    cm");
    OLED_ShowNum(2, 9, actualCm, 3);
	
	OLED_ShowString(3, 1, "L:");
	OLED_ShowSignedNum(3, 3, left, 4);
	OLED_ShowString(3, 9, "R:");
	OLED_ShowSignedNum(3, 11, right, 4);
	
    OLED_ShowString(4, 1, "K2: BACK");

    WaitKey2();
}

//物体测距
static void RunObjectRange(void)
{
    uint16_t distance;
    uint16_t total;
    uint16_t page = 0;
    uint8_t key;

    Motor_StopAll();
    ObjectCounter_Reset();

    OLED_Clear();
    OLED_ShowString(1, 1, "RIGHT OBJECTS");
    OLED_ShowString(2, 1, "COUNT:");
    OLED_ShowString(3, 1, "RIGHT:      cm");
    OLED_ShowString(4, 1, "K2: END LAP");

    /* 测试阶段：手推小车绕行，K2 表示走完一圈 */
    while (1)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);
            break;
        }

        distance = Ultrasonic_ReadCm();
        ObjectCounter_Update(distance);

        OLED_ShowNum(2, 8, ObjectCounter_GetCount(), 3);

        if (distance == ULTRASONIC_NO_ECHO)
            OLED_ShowString(3, 8, "----");
        else
            OLED_ShowNum(3, 8, distance, 4);

        Delay_ms(70);
    }

    total = ObjectCounter_GetCount();

    /* 一圈结束：K1 翻看各物体距离，K2 返回菜单 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "OBJECT RESULT");
        OLED_ShowString(2, 1, "COUNT:");
        OLED_ShowNum(2, 8, total, 3);

        if (total == 0)
        {
            OLED_ShowString(3, 1, "NO OBJECT");
        }
        else
        {
            OLED_ShowString(3, 1, "OBJ:   D:   cm");
            OLED_ShowNum(3, 5, page + 1, 2);
            OLED_ShowNum(3, 10,
                         ObjectCounter_GetDistance(page), 3);
        }

        OLED_ShowString(4, 1, "K1:NEXT K2:BACK");

        key = WaitKey();

        if (key == 1 && total > 0)
        {
            page++;
            if (page >= total || page >= OBJECT_MAX_RECORDS)
                page = 0;
        }
        else if (key == 2)
        {
            return;
        }
    }
}

/* 边循线边统计右侧物体，完成一圈后显示结果 */
static void RunObjectFollow(void)
{
    uint16_t distance = ULTRASONIC_NO_ECHO;
    uint16_t total;
    uint16_t page = 0;
    uint8_t key;
    uint8_t value;
    uint8_t x1Black;
    uint8_t x2Black;
    uint8_t x3Black;
    uint8_t x4Black;
    uint8_t cornerCount = 0;
    uint8_t objectSample = 0;
    uint8_t gapActive = 0;
    uint8_t turnActive = 0;
    uint8_t turnTicks = 0;
    int8_t turnDirection = 0;
    int8_t error;
    int8_t lastError = 0;
    int16_t correction;
    int16_t steering;
    int16_t ax, ay, az, gx, gy, gz;
    int32_t left;
    int32_t right;
    int32_t average;
    int32_t gapStart;
    int32_t gapTargetCounts;
    float gapHeading;
    float headingError;
    uint32_t startCycles;
    const int16_t basePwm = 22;
    const int16_t gapPwm = 20;
    const int16_t turnPwm = 22;

    Motor_StopAll();
    ObjectCounter_Reset();

    gapTargetCounts = (int32_t)(
        LINE_GAP_CM * WHEEL_COUNTS_PER_REV /
        (PI_VALUE * WHEEL_DIAMETER_CM) + 0.5f);

    Heading_Zero();
    startCycles = DWT_CYCCNT_REG;

    OLED_Clear();
    OLED_ShowString(1, 1, "OBJECT FOLLOW");
    OLED_ShowString(2, 1, "COUNT:");
    OLED_ShowNum(2, 8, 0, 3);
    OLED_ShowString(3, 1, "RIGHT:      cm");
    OLED_ShowString(3, 8, "----");
    OLED_ShowString(4, 1, "K2: END LAP");

    while (1)
    {
        /* K2 可以提前结束本圈并查看结果 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            Motor_StopAll();

            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);

            break;
        }

        value = LineSensor_ReadRaw();
        x2Black = ((value & 0x04) == 0);
        x1Black = ((value & 0x08) == 0);
        x3Black = ((value & 0x02) == 0);
        x4Black = ((value & 0x01) == 0);

        left = Encoder_GetLeftCount();
        right = Encoder_GetRightCount();
        average = (left + right) / 2;

        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        Heading_Update(gz);

        /*
         * 超声波不用每个控制周期都读取，避免阻塞循线控制。
         * 大约每4个控制周期测量一次，并跳过原地转弯阶段。
         */
        objectSample++;
        if (!turnActive && objectSample >= 4)
        {
            objectSample = 0;
            distance = Ultrasonic_ReadCm();
            ObjectCounter_Update(distance);

            OLED_ShowNum(2, 8, ObjectCounter_GetCount(), 3);

            if (distance == ULTRASONIC_NO_ECHO)
                OLED_ShowString(3, 8, "----");
            else
                OLED_ShowNum(3, 8, distance, 4);
        }

        /* 90度转角处理 */
        if (!turnActive)
        {
            if (x2Black && !x3Black && !x4Black)
            {
                turnActive = 1;
                turnDirection = -1;
                turnTicks = 0;
            }
            else if (x4Black && !x2Black && !x1Black)
            {
                turnActive = 1;
                turnDirection = 1;
                turnTicks = 0;
            }
        }

        if (turnActive)
        {
            if (turnDirection < 0)
                Motor_SetSides(-turnPwm, turnPwm);
            else
                Motor_SetSides(turnPwm, -turnPwm);

            if (turnTicks < 250)
                turnTicks++;

            if (turnTicks >= 5 && (x1Black || x3Black))
            {
                turnActive = 0;
                turnDirection = 0;
                turnTicks = 0;
                lastError = 0;
                cornerCount++;

                if (cornerCount >= 4)
                    break;
            }

            Delay_ms(20);
            continue;
        }

        /* 全白时进入60 cm断线段 */
        if (!x1Black && !x2Black && !x3Black && !x4Black)
        {
            if (!gapActive)
            {
                gapActive = 1;
                gapStart = average;
                gapHeading = Heading_GetDeg();
            }
        }

        if (gapActive)
        {
            if (average - gapStart >= gapTargetCounts)
            {
                gapActive = 0;
                lastError = 0;
            }
            else
            {
                headingError = Heading_GetDeg() - gapHeading;

                if (headingError >= 25.0f)
                {
                    Motor_SetSides(25, -25);
                }
                else if (headingError <= -25.0f)
                {
                    Motor_SetSides(-25, 25);
                }
                else
                {
                    if (headingError > -5.0f && headingError < 5.0f)
                        steering = 0;
                    else
                        steering = -(int16_t)(headingError * 1.0f);

                    if (stee.ring > 20)
                        steering = 20;
                    if (steering < -20)
                        steering = -20;

                    Motor_SetSides(gapPwm - steering,
                                   gapPwm + steering);
                }

                Delay_ms(20);
                continue;
            }
        }

        /* 普通循线 */
        error = LineSensor_GetError(value);

        if (error == 127)
            error = lastError;
        else
            lastError = error;

        correction = -(int16_t)error * 8;

        if (correction > 20)
            correction = 20;
        if (correction < -20)
            correction = -20;

        Motor_SetSides(basePwm - correction,
                       basePwm + correction);

        /* 一圈最长20秒，超时也停车显示当前结果 */
        if ((uint32_t)(DWT_CYCCNT_REG - startCycles) >
            SystemCoreClock * 20UL)
        {
            break;
        }

        Delay_ms(20);
    }

    Motor_StopAll();
    total = ObjectCounter_GetCount();

    /* K1翻看各物体最近距离，K2返回菜单 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "OBJECT RESULT");
        OLED_ShowString(2, 1, "COUNT:");
        OLED_ShowNum(2, 8, total, 3);

        if (total == 0)
        {
            OLED_ShowString(3, 1, "NO OBJECT");
        }
        else
        {
            OLED_ShowString(3, 1, "OBJ:   D:   cm");
            OLED_ShowNum(3, 5, page + 1, 2);
            OLED_ShowNum(3, 10,
                         ObjectCounter_GetDistance(page), 3);
        }

        OLED_ShowString(4, 1, "K1:NEXT K2:BACK");
        key = WaitKey();

        if (key == 1 && total > 0)
        {
            page++;
            if (page >= total || page >= OBJECT_MAX_RECORDS)
                page = 0;
        }
        else if (key == 2)
        {
            return;
        }
    }
}

//循线
static void RunLine(void)
{
    uint8_t value;
    uint8_t x1Black;
    uint8_t x2Black;
    uint8_t x3Black;
    uint8_t x4Black;
    int8_t error;
    int8_t lastError = 0;
    int16_t correction;
    int8_t turning = 0;       /* -1：左转，1：右转，0：普通循线 */
    uint8_t turnTicks = 0;
    const int16_t basePwm = 25;
    const int16_t turnPwm = 22;

    Motor_StopAll();

    OLED_Clear();
    OLED_ShowString(1, 1, "LINE FOLLOW");
    OLED_ShowString(3, 1, "K2: STOP");

    while (1)
    {
        /* K2 停止循线 */
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            Motor_StopAll();

            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);

            return;
        }

        value = LineSensor_ReadRaw();

        /* 实际排列：X2、X1、X3、X4；黑线为低电平 */
        x2Black = ((value & 0x04) == 0);
        x1Black = ((value & 0x08) == 0);
        x3Black = ((value & 0x02) == 0);
        x4Black = ((value & 0x01) == 0);

        /*
         * 外侧探头单独压到黑线，认为遇到 90 度转角。
         * 原地转向，避免高速画弧线冲出轨道。
         */
        if (turning == 0)
        {
            if (x2Black && !x3Black && !x4Black)
            {
                turning = -1;
                turnTicks = 0;
            }
            else if (x4Black && !x2Black && !x1Black)
            {
                turning = 1;
                turnTicks = 0;
            }
        }

        if (turning < 0)
        {
            /* 左转：左轮反转，右轮正转 */
            Motor_SetSides(-turnPwm, turnPwm);
            turnTicks++;

            /* 至少转动一小段时间后，中间探头重新压到黑线 */
            if (turnTicks >= 5 && (x1Black || x3Black))
            {
                turning = 0;
                turnTicks = 0;
                lastError = 0;
            }

            Delay_ms(20);
            continue;
        }

        if (turning > 0)
        {
            /* 右转：左轮正转，右轮反转 */
            Motor_SetSides(turnPwm, -turnPwm);
            turnTicks++;

            /* 至少转动一小段时间后，中间探头重新压到黑线 */
            if (turnTicks >= 5 && (x1Black || x3Black))
            {
                turning = 0;
                turnTicks = 0;
                lastError = 0;
            }

            Delay_ms(20);
            continue;
        }

        error = LineSensor_GetError(value);

        if (error == 127)
        {
            /*
             * 全部为白色，暂时保持上一次修正方向。
             * 这里还没有加入断线直行功能。
             */
            error = lastError;
        }
        else
        {
            lastError = error;
        }

        /*
         * error < 0：黑线偏左，需要向左修正
         * error > 0：黑线偏右，需要向右修正
         */
        correction = -(int16_t)error * 8;

        if (correction > 20)
            correction = 20;

        if (correction < -20)
            correction = -20;

        /*
         * 正 correction：右轮快、左轮慢，小车向左转
         * 负 correction：左轮快、右轮慢，小车向右转
         */
        Motor_SetSides(basePwm - correction, basePwm + correction);

        OLED_ShowNum(2, 1,  (value >> 2) & 1, 1);  // X2
        OLED_ShowNum(2, 4,  (value >> 3) & 1, 1);  // X1
        OLED_ShowNum(2, 7,  (value >> 1) & 1, 1);  // X3
        OLED_ShowNum(2, 10, value & 1, 1);         // X4

        Delay_ms(20);
    }
}

/*
 * 循线考核版：
 * 1. K1 选择 1～5 圈，K2 开始；
 * 2. 外侧探头压线时原地转过 90 度；
 * 3. 四路全白时，按编码器走 60 cm，并用 MPU6050 保持方向；
 * 4. 每完成 4 个转角计为 1 圈。
 */
static void RunLineCourse(void)
{
    uint8_t key;
    uint8_t targetLaps = 1;
    uint8_t cornerCount = 0;
    uint8_t refresh = 0;
    uint8_t success = 0;
    uint8_t value;
    uint8_t x1Black;
    uint8_t x2Black;
    uint8_t x3Black;
    uint8_t x4Black;
    uint8_t gapActive = 0;
    uint8_t turnActive = 0;
    uint8_t turnTicks = 0;
    int8_t turnDirection = 0;       /* -1：左转，1：右转 */
    int8_t error;
    int8_t lastError = 0;
    int16_t correction;
    int16_t steering;
    int16_t ax, ay, az, gx, gy, gz;
    int32_t left;
    int32_t right;
    int32_t average;
    int32_t gapStart;
    int32_t gapTargetCounts;
    float gapHeading;
    float headingError;
    uint32_t startCycles;
    const int16_t basePwm = 22;
    const int16_t turnPwm = 22;
    const int16_t gapPwm = 20;

    /* K1 选择圈数，K2 开始 */
    while (1)
    {
        OLED_Clear();
        OLED_ShowString(1, 1, "SET LINE LAPS");
        OLED_ShowString(2, 1, "LAPS:");
        OLED_ShowNum(2, 7, targetLaps, 1);
        OLED_ShowString(3, 1, "K1: +1 LAP");
        OLED_ShowString(4, 1, "K2: START");

        key = WaitKey();

        if (key == 1)
        {
            targetLaps++;
            if (targetLaps > 5)
                targetLaps = 1;
        }
        else if (key == 2)
        {
            break;
        }
    }

    gapTargetCounts = (int32_t)(
        LINE_GAP_CM * WHEEL_COUNTS_PER_REV /
        (PI_VALUE * WHEEL_DIAMETER_CM) + 0.5f);

    Heading_Zero();
    startCycles = DWT_CYCCNT_REG;

    OLED_Clear();
    OLED_ShowString(1, 1, "LINE LAPS:");
    OLED_ShowNum(1, 11, targetLaps, 1);
    OLED_ShowString(2, 1, "X2 X1 X3 X4");
    OLED_ShowString(3, 1, "CORNERS:");
    OLED_ShowNum(3, 10, cornerCount, 2);
    OLED_ShowString(4, 1, "K2: STOP");

    while (1)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
        {
            Motor_StopAll();

            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == Bit_RESET)
                Delay_ms(10);

            return;
        }

        value = LineSensor_ReadRaw();
        x2Black = ((value & 0x04) == 0);
        x1Black = ((value & 0x08) == 0);
        x3Black = ((value & 0x02) == 0);
        x4Black = ((value & 0x01) == 0);

        left = Encoder_GetLeftCount();
        right = Encoder_GetRightCount();
        average = (left + right) / 2;

        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        Heading_Update(gz);

        /* 90 度转弯：外侧探头压线后原地旋转 */
        if (!turnActive)
        {
            if (x2Black && !x3Black && !x4Black)
            {
                turnActive = 1;
                turnDirection = -1;
                turnTicks = 0;
            }
            else if (x4Black && !x2Black && !x1Black)
            {
                turnActive = 1;
                turnDirection = 1;
                turnTicks = 0;
            }
        }

        if (turnActive)
        {
            if (turnDirection < 0)
                Motor_SetSides(-turnPwm, turnPwm);
            else
                Motor_SetSides(turnPwm, -turnPwm);

            if (turnTicks < 250)
                turnTicks++;

            /* 至少转约100 ms，防止刚触发就退出 */
            if (turnTicks >= 5 && (x1Black || x3Black))
            {
                turnActive = 0;
                turnDirection = 0;
                turnTicks = 0;
                lastError = 0;
                cornerCount++;

                /* 每完成一圈重新计时，符合单圈20秒的题目限制 */
                if ((cornerCount % 4) == 0)
                    startCycles = DWT_CYCCNT_REG;

                if (cornerCount >= targetLaps * 4)
                {
                    success = 1;
                    break;
                }
            }

            Delay_ms(20);
            continue;
        }

        /* 全白表示进入空白段，记录进入时的位置和方向 */
        if (!x1Black && !x2Black && !x3Black && !x4Black)
        {
            if (!gapActive)
            {
                gapActive = 1;
                gapStart = average;
                gapHeading = Heading_GetDeg();
            }
        }

        if (gapActive)
        {
            if (average - gapStart >= gapTargetCounts)
            {
                gapActive = 0;
                lastError = 0;
            }
            else
            {
                /*
                 * 空白段内降低速度，只用 MPU6050 保持进入时的方向。
                 * 控制参数与已经验证过的自动纠偏模式一致：
                 * 小偏差差速修正，大偏差原地转回。
                 */
                headingError = Heading_GetDeg() - gapHeading;

                if (headingError >= 25.0f)
                {
                    /* 当前车头偏左，向右原地转回 */
                    Motor_SetSides(25, -25);
                }
                else if (headingError <= -25.0f)
                {
                    /* 当前车头偏右，向左原地转回 */
                    Motor_SetSides(-25, 25);
                }
                else
                {
                    if (headingError > -5.0f && headingError < 5.0f)
                        steering = 0;
                    else
                        steering = -(int16_t)(headingError * 1.0f);

                    if (steering > 20)
                        steering = 20;
                    if (steering < -20)
                        steering = -20;

                    Motor_SetSides(gapPwm - steering,
                                   gapPwm + steering);
                }

                Delay_ms(20);
                continue;
            }
        }

        /* 普通黑线循迹 */
        error = LineSensor_GetError(value);

        if (error == 127)
            error = lastError;
        else
            lastError = error;

        correction = -(int16_t)error * 8;

        if (correction > 20)
            correction = 20;
        if (correction < -20)
            correction = -20;

        Motor_SetSides(basePwm - correction,
                       basePwm + correction);

        refresh++;
        if (refresh >= 5)
        {
            refresh = 0;
            OLED_ShowNum(2, 1,  (value >> 2) & 1, 1);
            OLED_ShowNum(2, 4,  (value >> 3) & 1, 1);
            OLED_ShowNum(2, 7,  (value >> 1) & 1, 1);
            OLED_ShowNum(2, 10, value & 1, 1);
            OLED_ShowNum(3, 10, cornerCount, 2);
        }

        if ((uint32_t)(DWT_CYCCNT_REG - startCycles) >
            SystemCoreClock * 20UL)
        {
            break;
        }

        Delay_ms(20);
    }

    Motor_StopAll();

    OLED_Clear();
    OLED_ShowString(1, 1, success ? "LINE DONE" : "LINE STOP");
    OLED_ShowString(2, 1, "LAPS:");
    OLED_ShowNum(2, 7, cornerCount / 4, 2);
    OLED_ShowString(4, 1, "K2: BACK");
    WaitKey2();
}

void Mode_Run(CarMode mode)
{
    Motor_StopAll();

    switch (mode)
    {
        case MODE_HOLD:      //自动纠偏
            RunHold();
            break;

        case MODE_ANGLE:	 //指定角度
            RunAngle();
            break;

        case MODE_DISTANCE:	 //里程计
			RunOdometry();
			break;

		case MODE_LINE:		 //循线
			RunLineCourse();
			break;

        case MODE_OBJECT:    //物体测距
            RunObjectFollow();
            break;

        default:
            break;
    }

    Motor_StopAll();
}
