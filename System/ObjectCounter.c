/*
*本模块用于接收超声波模块信号以计数
*/

#include "ObjectCounter.h"
#include "Ultrasonic.h"

#define OBJECT_DETECT_CM   30
#define OBJECT_RELEASE_CM  35

/*
 * 探头相对循迹线的左右偏移，单位 cm：
 * 探头在线的右侧填正数，在线的左侧填负数。
 * 尚未测量时先填 0，此时记录的是探头到物体的距离。
 */
#define SENSOR_TO_LINE_OFFSET_CM  0    //记录偏移量，消除车身偏离的误差

static uint16_t distances[OBJECT_MAX_RECORDS];  //存储所有的距离数据 
static uint16_t count = 0;                      
static uint16_t candidateMin = 0;               //候选最小
static uint8_t detecting = 0;
static uint8_t nearTimes = 0;
static uint8_t farTimes = 0;


//计数重置
void ObjectCounter_Reset(void)  
{
    count = 0;
    candidateMin = 0;
    detecting = 0;
    nearTimes = 0;
    farTimes = 0;
}

//
void ObjectCounter_Update(uint16_t sensorCm)
{
    uint16_t distance;
    int32_t corrected;

    if (sensorCm == ULTRASONIC_NO_ECHO)
    {
        distance = ULTRASONIC_NO_ECHO;
    }
    else
    {
        corrected = (int32_t)sensorCm + SENSOR_TO_LINE_OFFSET_CM;
        if (corrected < 0)
            corrected = 0;

        distance = (uint16_t)corrected;
    }

    if (!detecting)    
    {
        /* 连续两次进入 30 cm 范围，确认发现一个物体 */
        if (distance <= OBJECT_DETECT_CM)
        {
            if (nearTimes == 0 || distance < candidateMin)
                candidateMin = distance;

            nearTimes++;

            if (nearTimes >= 2)  //检测到两次
            {
                detecting = 1;
                nearTimes = 0;
                farTimes = 0;

                count++;
                if (count <= OBJECT_MAX_RECORDS)
                    distances[count - 1] = candidateMin;  //存入最近距离
            }
        }
        else
        {
            nearTimes = 0;
        }
    }
    else
    {
        /* 经过物体期间，保留测到的最近距离 */
        if (count <= OBJECT_MAX_RECORDS &&
            distance != ULTRASONIC_NO_ECHO &&
            distance < distances[count - 1])
        {
            distances[count - 1] = distance;
        }

        /* 连续三次远离物体，才准备识别下一个 */
        if (distance == ULTRASONIC_NO_ECHO ||
            distance >= OBJECT_RELEASE_CM)       //测不到物体或者>30cm
        {
            farTimes++;

            if (farTimes >= 3)
            {
                detecting = 0;
                farTimes = 0;
            }
        }
        else
        {
            farTimes = 0;
        }
    }
}

uint16_t ObjectCounter_GetCount(void)
{
    return count;
}

uint16_t ObjectCounter_GetDistance(uint16_t index)
{
    if (index >= count || index >= OBJECT_MAX_RECORDS)
        return ULTRASONIC_NO_ECHO;

    return distances[index];
}
