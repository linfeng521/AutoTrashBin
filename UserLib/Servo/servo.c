#include "servo.h"

void set_degree(uint8_t degree)
{
    // 限制角度范围 0-180
    if (degree > 180)
        degree = 180;
    // 0~180° -> 占空比2.5%~12.5% -> 比较器 5~25
    uint16_t duration = (uint16_t)degree * 20 / 180 + 5;
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, duration);
    
}
