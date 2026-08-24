#include "motor.h"
#include "main.h"
#include "pwm.h"
#include "tim.h"

/*
 * Existing, confirmed wiring:
 * Left:  IN1 = PB13, IN2 = PB14, PWM = TIM3 CH1 (PA6)
 * Right: IN3 = PB15, IN4 = PB1,  PWM = TIM3 CH2 (PA7)
 *
 * Positive speed uses the same electrical direction as the user's
 * previously working Motor_Forward(). Negative speed reverses it.
 */

#define MOTOR_MAX_SPEED 1000
#define CURVE_INNER_DIVISOR 4U

static int8_t leftDirection;
static int8_t rightDirection;

static int16_t Motor_Clamp(int16_t speed)
{
    if (speed > MOTOR_MAX_SPEED)
    {
        return MOTOR_MAX_SPEED;
    }

    if (speed < -MOTOR_MAX_SPEED)
    {
        return -MOTOR_MAX_SPEED;
    }

    return speed;
}

static void Motor_SetLeftDirection(int8_t direction)
{
    if (direction > 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    }
    else if (direction < 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    }
}

static void Motor_SetRightDirection(int8_t direction)
{
    if (direction > 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }
    else if (direction < 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }
}

static void Motor_SetLeft(int16_t speed)
{
    int8_t newDirection;
    uint16_t magnitude;

    speed = Motor_Clamp(speed);
    newDirection = (speed > 0) ? 1 : ((speed < 0) ? -1 : 0);
    magnitude = (uint16_t)((speed < 0) ? -speed : speed);

    if ((leftDirection != 0) &&
        (newDirection != 0) &&
        (leftDirection != newDirection))
    {
        PWM_SetLeftSpeed(0U);
    }

    Motor_SetLeftDirection(newDirection);
    PWM_SetLeftSpeed(magnitude);
    leftDirection = newDirection;
}

static void Motor_SetRight(int16_t speed)
{
    int8_t newDirection;
    uint16_t magnitude;

    speed = Motor_Clamp(speed);
    newDirection = (speed > 0) ? 1 : ((speed < 0) ? -1 : 0);
    magnitude = (uint16_t)((speed < 0) ? -speed : speed);

    if ((rightDirection != 0) &&
        (newDirection != 0) &&
        (rightDirection != newDirection))
    {
        PWM_SetRightSpeed(0U);
    }

    Motor_SetRightDirection(newDirection);
    PWM_SetRightSpeed(magnitude);
    rightDirection = newDirection;
}

void Motor_Init(void)
{
    leftDirection = 0;
    rightDirection = 0;
    PWM_Init(&htim3);
    Motor_Stop();
}

void Motor_Set(int16_t leftSpeed, int16_t rightSpeed)
{
    Motor_SetLeft(rightSpeed);
    Motor_SetRight(leftSpeed);
}

void Motor_Forward(uint16_t speed)
{
    Motor_Set((int16_t)speed, (int16_t)speed);
}

void Motor_Backward(uint16_t speed)
{
    Motor_Set(-(int16_t)speed, -(int16_t)speed);
}

void Motor_CurveLeft(uint16_t speed)
{
    Motor_Set((int16_t)(speed / CURVE_INNER_DIVISOR), (int16_t)speed);
}

void Motor_CurveRight(uint16_t speed)
{
    Motor_Set((int16_t)speed, (int16_t)(speed / CURVE_INNER_DIVISOR));
}

void Motor_PivotLeft(uint16_t speed)
{
    Motor_Set(-(int16_t)speed, (int16_t)speed);
}

void Motor_PivotRight(uint16_t speed)
{
    Motor_Set((int16_t)speed, -(int16_t)speed);
}

void Motor_Left(uint16_t speed)
{
    Motor_PivotLeft(speed);
}

void Motor_Right(uint16_t speed)
{
    Motor_PivotRight(speed);
}

void Motor_Stop(void)
{
    Motor_Set(0, 0);
}
