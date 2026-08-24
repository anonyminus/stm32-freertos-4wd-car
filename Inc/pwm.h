
#ifndef PWM_H
#define PWM_H

#include "stm32f4xx_hal.h"

void PWM_Init(TIM_HandleTypeDef *timer);
void PWM_SetLeftSpeed(uint16_t speed);
void PWM_SetRightSpeed(uint16_t speed);
void PWM_SetSpeed(uint16_t leftSpeed, uint16_t rightSpeed);

#endif
