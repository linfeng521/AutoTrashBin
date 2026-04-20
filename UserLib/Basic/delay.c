#include "tim.h" //拿到 htim2
#include "delay.h"
extern TIM_HandleTypeDef htim2;

/* 微秒延时 */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);   // 清零计数器

    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}