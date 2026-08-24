#include "pwm.h"
#include "main.h"
#include "tim.h"

#define PWM_MAX_COMPARE 1000U

static uint16_t PWM_Limit(uint16_t speed)
{
    if (speed > PWM_MAX_COMPARE)
    {
        return PWM_MAX_COMPARE;
    }

    return speed;
}

void PWM_Init(TIM_HandleTypeDef *timer)
{
    if (HAL_TIM_PWM_Start(timer, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Start(timer, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }

    PWM_SetSpeed(0U, 0U);
}

void PWM_SetLeftSpeed(uint16_t speed)
{
    __HAL_TIM_SET_COMPARE(
        &htim3,
        TIM_CHANNEL_1,
        PWM_Limit(speed)
    );
}

void PWM_SetRightSpeed(uint16_t speed)
{
    __HAL_TIM_SET_COMPARE(
        &htim3,
        TIM_CHANNEL_2,
        PWM_Limit(speed)
    );
}

void PWM_SetSpeed(uint16_t leftSpeed, uint16_t rightSpeed)
{
    PWM_SetLeftSpeed(leftSpeed);
    PWM_SetRightSpeed(rightSpeed);
}
