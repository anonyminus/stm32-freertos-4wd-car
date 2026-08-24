#include "auto_drive.h"
#include "car_config.h"
#include "main.h"
#include "mode.h"
#include "motor.h"
#include "ultrasonic.h"
#include "cmsis_os.h"

#define SPEED_TO_PWM(percent) ((uint16_t)((percent) * 10U))
#define MM_TO_CM(mm)          ((float)(mm) / 10.0f)
#define CENTER_DEADBAND_CM             1.5f
#define CENTER_KP_PWM_PER_CM           10.0f
#define CENTER_MAX_CORRECTION_PWM      110

volatile float centerErrorTestCm = 0.0f;
volatile int16_t centerLeftPwmTest = 0;
volatile int16_t centerRightPwmTest = 0;

typedef enum
{
    AUTO_FORWARD = 0,
    AUTO_STOP_PAUSE,
    AUTO_REVERSE,
    AUTO_PIVOT_LEFT,
    AUTO_PIVOT_RIGHT
} AutoState_t;

static uint8_t SensorFresh(
    const UltrasonicData_t *data,
    UltrasonicSensor_t sensor,
    uint32_t now)
{
    return (data->valid[sensor] != 0U) &&
           ((now - data->updatedMs[sensor]) <= ULTRASONIC_STALE_MS);
}

static void DriveCentered(float leftCm, float rightCm)
{
    int16_t basePwm =
        (int16_t)SPEED_TO_PWM(DRIVE_SPEED);

    float errorCm = rightCm - leftCm;
    int16_t correctionPwm = 0;

    if ((errorCm > CENTER_DEADBAND_CM) ||
        (errorCm < -CENTER_DEADBAND_CM))
    {
        correctionPwm =
            (int16_t)(errorCm * CENTER_KP_PWM_PER_CM);
    }

    if (correctionPwm > CENTER_MAX_CORRECTION_PWM)
    {
        correctionPwm = CENTER_MAX_CORRECTION_PWM;
    }
    else if (correctionPwm < -CENTER_MAX_CORRECTION_PWM)
    {
        correctionPwm = -CENTER_MAX_CORRECTION_PWM;
    }

    int16_t leftPwm = basePwm + correctionPwm;
    int16_t rightPwm = basePwm - correctionPwm;

    centerErrorTestCm = errorCm;
    centerLeftPwmTest = leftPwm;
    centerRightPwmTest = rightPwm;

    Motor_Set(leftPwm, rightPwm);
}

void AutoDriveTask(void *argument)
{
    (void)argument;

    AutoState_t state = AUTO_FORWARD;
    uint32_t stateStarted = HAL_GetTick();
    uint32_t lastJunctionTurn = HAL_GetTick();
    uint8_t preferredLeft = 1U;

    for (;;)
    {
        UltrasonicData_t sensors;
        uint32_t now = HAL_GetTick();

        if (Mode_Get() != MODE_AUTO)
        {
            state = AUTO_FORWARD;
            stateStarted = now;
            lastJunctionTurn = now;
            osDelay(30U);
            continue;
        }

        Ultrasonic_Copy(&sensors);

        uint8_t frontValid = SensorFresh(
            &sensors,
            ULTRASONIC_FRONT,
            now
        );
        uint8_t leftValid = SensorFresh(
            &sensors,
            ULTRASONIC_LEFT,
            now
        );
        uint8_t rightValid = SensorFresh(
            &sensors,
            ULTRASONIC_RIGHT,
            now
        );

        float front = sensors.distanceCm[ULTRASONIC_FRONT];
        float left = sensors.distanceCm[ULTRASONIC_LEFT];
        float right = sensors.distanceCm[ULTRASONIC_RIGHT];

        if (frontValid == 0U)
        {
            Motor_Stop();
            osDelay(30U);
            continue;
        }

        switch (state)
        {
            case AUTO_FORWARD:
                if (front <= MM_TO_CM(FRONT_STOP_MM))
                {
                    Motor_Stop();

                    if (leftValid && rightValid)
                    {
                        preferredLeft = (left >= right);
                    }
                    else
                    {
                        preferredLeft = leftValid;
                    }

                    state = AUTO_STOP_PAUSE;
                    stateStarted = now;
                }
                else if (((now - lastJunctionTurn) >= JUNCTION_COOLDOWN_MS) &&
                         leftValid && rightValid &&
                         (left >= MM_TO_CM(SIDE_OPEN_MM)) &&
                         (left > (front + MM_TO_CM(JUNCTION_ADVANTAGE_MM))) &&
                         (left > right))
                {
                    Motor_PivotLeft(SPEED_TO_PWM(TURN_SPEED));
                    state = AUTO_PIVOT_LEFT;
                    stateStarted = now;
                    lastJunctionTurn = now;
                }
                else if (((now - lastJunctionTurn) >= JUNCTION_COOLDOWN_MS) &&
                         leftValid && rightValid &&
                         (right >= MM_TO_CM(SIDE_OPEN_MM)) &&
                         (right > (front + MM_TO_CM(JUNCTION_ADVANTAGE_MM))) &&
                         (right > left))
                {
                    Motor_PivotRight(SPEED_TO_PWM(TURN_SPEED));
                    state = AUTO_PIVOT_RIGHT;
                    stateStarted = now;
                    lastJunctionTurn = now;
                }
                else if (leftValid && rightValid)
                {
                    DriveCentered(left, right);
                }
                else
                {
                    Motor_Forward(SPEED_TO_PWM(DRIVE_SPEED));
                }
                break;

            case AUTO_STOP_PAUSE:
                Motor_Stop();

                if ((now - stateStarted) >= STOP_PAUSE_MS)
                {
                    uint8_t leftBlocked =
                        (leftValid == 0U) ||
                        (left < MM_TO_CM(SIDE_BLOCKED_MM));
                    uint8_t rightBlocked =
                        (rightValid == 0U) ||
                        (right < MM_TO_CM(SIDE_BLOCKED_MM));

                    if (leftBlocked && rightBlocked)
                    {
                        Motor_Backward(SPEED_TO_PWM(REVERSE_SPEED));
                        state = AUTO_REVERSE;
                    }
                    else if (preferredLeft != 0U)
                    {
                        Motor_PivotLeft(SPEED_TO_PWM(TURN_SPEED));
                        state = AUTO_PIVOT_LEFT;
                    }
                    else
                    {
                        Motor_PivotRight(SPEED_TO_PWM(TURN_SPEED));
                        state = AUTO_PIVOT_RIGHT;
                    }

                    stateStarted = now;
                }
                break;

            case AUTO_REVERSE:
                Motor_Backward(SPEED_TO_PWM(REVERSE_SPEED));

                if ((now - stateStarted) >= REVERSE_TIME_MS)
                {
                    if (leftValid && rightValid)
                    {
                        preferredLeft = (left >= right);
                    }
                    else
                    {
                        preferredLeft = leftValid;
                    }

                    Motor_Stop();

                    if (preferredLeft != 0U)
                    {
                        Motor_PivotLeft(SPEED_TO_PWM(TURN_SPEED));
                        state = AUTO_PIVOT_LEFT;
                    }
                    else
                    {
                        Motor_PivotRight(SPEED_TO_PWM(TURN_SPEED));
                        state = AUTO_PIVOT_RIGHT;
                    }

                    stateStarted = now;
                }
                break;

            case AUTO_PIVOT_LEFT:
                Motor_PivotLeft(SPEED_TO_PWM(TURN_SPEED));

                if ((((now - stateStarted) >= TURN_MIN_TIME_MS) &&
                     (front >= MM_TO_CM(FRONT_CLEAR_MM))) ||
                    ((now - stateStarted) >= TURN_MAX_TIME_MS))
                {
                    Motor_Stop();
                    state = AUTO_FORWARD;
                    stateStarted = now;
                    lastJunctionTurn = now;
                }
                break;

            case AUTO_PIVOT_RIGHT:
                Motor_PivotRight(SPEED_TO_PWM(TURN_SPEED));

                if ((((now - stateStarted) >= TURN_MIN_TIME_MS) &&
                     (front >= MM_TO_CM(FRONT_CLEAR_MM))) ||
                    ((now - stateStarted) >= TURN_MAX_TIME_MS))
                {
                    Motor_Stop();
                    state = AUTO_FORWARD;
                    stateStarted = now;
                    lastJunctionTurn = now;
                }
                break;

            default:
                Motor_Stop();
                state = AUTO_FORWARD;
                stateStarted = now;
                lastJunctionTurn = now;
                break;
        }

        osDelay(30U);
    }
}
