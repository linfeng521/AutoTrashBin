#ifndef __SERVO_H__
#define __SERVO_H__

#include "tim.h"

#define SERVO_PWM_CHANNEL PWM_CHANNEL_1
void set_degree(uint8_t degree);
#endif
