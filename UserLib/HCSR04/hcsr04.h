#ifndef __HCSR04_H
#define __HCSR04_H

#include "tim.h"
#include "gpio.h"

// 函数声明
void HCSR04_Trigger(void);
float HCSR04_GetDistance(void);

#endif
