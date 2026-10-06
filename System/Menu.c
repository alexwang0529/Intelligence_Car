/*
*本模块用于管理一个模式菜单
*通过按键选择不同模式并执行
*/

#include "stm32f10x.h"
#include "Delay.h"
#include "Key.h"
#include "OLED.h"
#include "Motor.h"
#include "Menu.h"

static char *modeNames[MODE_COUNT] =      //数组存放多个模式
{
    "DIRECTION HOLD",
    "ANGLE TURN",
    "DISTANCE",
    "LINE FOLLOW",
    "OBJECT RANGE"
};

/*
*显示模式选择界面，显示对应数字的模式
*/
static void Menu_Show(uint8_t selected)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "SELECT MODE");
    OLED_ShowNum(1, 13, selected + 1, 1);
    OLED_ShowString(1, 14, "/5");
    OLED_ShowString(2, 1, modeNames[selected]);  //显示模式名称
    OLED_ShowString(3, 1, "K1: NEXT");
    OLED_ShowString(4, 1, "K2: ENTER");
}

//控制模式选择
CarMode Menu_Select(void)
{
    uint8_t selected = 0;
    uint8_t key;

    Motor_StopAll();
    Menu_Show(selected);

    while (1)
    {
        key = Key_GetNum();
 
        if (key == 1)                     //按下K1
        {
            selected++;					  //NEXT
            if (selected >= MODE_COUNT)   //Mode_Count在枚举中，==5
                selected = 0;

            Menu_Show(selected);		  //显示模式
        }
        else if (key == 2)
        {
            return (CarMode)selected;     //返回选择的模式，通过Mode_Run决定运行该模式
        }

        Delay_ms(20);
    }
}
