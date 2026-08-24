#include "manual_drive.h"
#include "car_config.h"
#include "main.h"
#include "mode.h"
#include "motor.h"
#include "wifi.h"
#include "cmsis_os.h"

#define SPEED_TO_PWM(percent) ((uint16_t)((percent) * 10U))

#define MANUAL_START_SPEED SPEED_TO_PWM(MANUAL_DRIVE_SPEED)
#define MANUAL_MIN_SPEED 300U
#define MANUAL_MAX_SPEED 900U
#define MANUAL_SPEED_STEP 100U
#define MANUAL_COMMAND_TIMEOUT_MS BLUETOOTH_COMMAND_TIMEOUT_MS

typedef enum
{
    MOTION_STOP = 0,
    MOTION_FORWARD,
    MOTION_BACKWARD,
    MOTION_LEFT,
    MOTION_RIGHT
} ManualMotion_t;

static void Manual_ApplyMotion(ManualMotion_t motion, uint16_t speed)
{
    switch (motion)
    {
        case MOTION_FORWARD:
            Motor_Forward(speed);
            break;

        case MOTION_BACKWARD:
            Motor_Backward(speed);
            break;

        case MOTION_LEFT:
            Motor_PivotLeft(SPEED_TO_PWM(MANUAL_TURN_SPEED));
            break;

        case MOTION_RIGHT:
            Motor_PivotRight(SPEED_TO_PWM(MANUAL_TURN_SPEED));
            break;

        case MOTION_STOP:
        default:
            Motor_Stop();
            break;
    }
}

void ManualDriveTask(void *argument)
{
    (void)argument;

    uint16_t speed = MANUAL_START_SPEED;
    uint32_t lastSequence = 0U;
    ManualMotion_t motion = MOTION_STOP;

    for (;;)
    {
        WifiManualState_t state;

        if (Mode_Get() != MODE_MANUAL)
        {
            motion = MOTION_STOP;
            osDelay(20U);
            continue;
        }

        Wifi_GetManualState(&state);

        if (state.sequence != lastSequence)
        {
            lastSequence = state.sequence;

            switch (state.command)
            {
                case MANUAL_CMD_FORWARD:
                    motion = MOTION_FORWARD;
                    break;

                case MANUAL_CMD_BACKWARD:
                    motion = MOTION_BACKWARD;
                    break;

                case MANUAL_CMD_LEFT:
                    motion = MOTION_LEFT;
                    break;

                case MANUAL_CMD_RIGHT:
                    motion = MOTION_RIGHT;
                    break;

                case MANUAL_CMD_SPEED_UP:
                    if (speed <= (MANUAL_MAX_SPEED - MANUAL_SPEED_STEP))
                    {
                        speed += MANUAL_SPEED_STEP;
                    }
                    break;

                case MANUAL_CMD_SPEED_DOWN:
                    if (speed >= (MANUAL_MIN_SPEED + MANUAL_SPEED_STEP))
                    {
                        speed -= MANUAL_SPEED_STEP;
                    }
                    break;

                case MANUAL_CMD_STOP:
                default:
                    motion = MOTION_STOP;
                    break;
            }
        }

        if ((HAL_GetTick() - state.updatedMs) >
            MANUAL_COMMAND_TIMEOUT_MS)
        {
            motion = MOTION_STOP;
        }

        Manual_ApplyMotion(motion, speed);
        osDelay(20U);
    }
}
