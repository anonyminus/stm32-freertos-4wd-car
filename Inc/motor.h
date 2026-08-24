
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx_hal.h"

void Motor_Init(void);

void Motor_Set(int16_t leftSpeed, int16_t rightSpeed);

void Motor_Forward(uint16_t speed);
void Motor_Backward(uint16_t speed);

void Motor_CurveLeft(uint16_t speed);
void Motor_CurveRight(uint16_t speed);

void Motor_PivotLeft(uint16_t speed);
void Motor_PivotRight(uint16_t speed);

void Motor_Left(uint16_t speed);
void Motor_Right(uint16_t speed);

void Motor_Stop(void);

#endif /* MOTOR_H */
