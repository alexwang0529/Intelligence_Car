#ifndef __MENU_H
#define __MENU_H

typedef enum          //枚举类型
{
    MODE_HOLD = 0,
    MODE_ANGLE,
    MODE_DISTANCE,
    MODE_LINE,
    MODE_OBJECT,
    MODE_COUNT
} CarMode;

CarMode Menu_Select(void);

#endif
