#include "hcsr04.h"

// 超声波测距相关变量（私有，只在本文件内可见）
static uint32_t upEdge = 0;   // 上升沿捕获的时间（us）
static uint32_t downEdge = 0; // 下降沿捕获的时间（us）
static volatile float distance = 0.0;  // 测量距离（cm）
static volatile uint8_t data_ready = 0; // 数据就绪标志

/**
 * @brief 触发超声波测距
 */
void HCSR04_Trigger(void)
{
    // 清除数据就绪标志
    data_ready = 0;

    // 发送 10us 高电平触发信号
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
    // 使用定时器延时 10us（更精确）
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < 10); // 等待 10us
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);

    // 清零定时器计数器，准备捕获
    __HAL_TIM_SET_COUNTER(&htim2, 0);
}

/**
 * @brief 获取超声波测距结果
 * @return 距离值（cm），如果数据无效返回 -1
 */
float HCSR04_GetDistance(void)
{
    if (data_ready) {
        data_ready = 0;  // 清除标志
        return distance;
    }
    return -1.0;  // 数据无效
}

/**
 * @brief 定时器输入捕获中断回调函数
 * @note 在中断中计算距离
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        upEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        downEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        // 计算距离：时间差 * 声速(0.034cm/us) / 2（来回）
        distance = (float)((downEdge - upEdge) * 0.034 / 2);
        data_ready = 1;  // 标记数据有效
    }
}
