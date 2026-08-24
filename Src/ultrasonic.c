#include "ultrasonic.h"
#include "car_config.h"
#include "main.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define ULTRASONIC_MIN_CM ((float)ULTRASONIC_MIN_MM / 10.0f)
#define ULTRASONIC_MAX_CM ((float)ULTRASONIC_MAX_MM / 10.0f)

typedef struct
{
    GPIO_TypeDef *trigPort;
    uint16_t trigPin;
    GPIO_TypeDef *echoPort;
    uint16_t echoPin;
    volatile uint32_t echoStartUs;
    volatile uint32_t echoPulseUs;
    volatile uint8_t waitingForFall;
    volatile uint8_t measurementReady;
} UltrasonicRuntime_t;

static UltrasonicRuntime_t sensors[ULTRASONIC_COUNT] = {
    {
        US_FRONT_TRIG_GPIO_Port, US_FRONT_TRIG_Pin,
        US_FRONT_ECHO_GPIO_Port, US_FRONT_ECHO_Pin,
        0U, 0U, 0U, 0U
    },
    {
        US_LEFT_TRIG_GPIO_Port, US_LEFT_TRIG_Pin,
        US_LEFT_ECHO_GPIO_Port, US_LEFT_ECHO_Pin,
        0U, 0U, 0U, 0U
    },
    {
        US_RIGHT_TRIG_GPIO_Port, US_RIGHT_TRIG_Pin,
        US_RIGHT_ECHO_GPIO_Port, US_RIGHT_ECHO_Pin,
        0U, 0U, 0U, 0U
    }
};

static UltrasonicData_t sharedData;

volatile float frontDistanceTestCm = -1.0f;
volatile float leftDistanceTestCm = -1.0f;
volatile float rightDistanceTestCm = -1.0f;

static void DelayUs(uint32_t delayUs)
{
    uint32_t startUs = __HAL_TIM_GET_COUNTER(&htim2);

    while ((__HAL_TIM_GET_COUNTER(&htim2) - startUs) < delayUs)
    {
        __NOP();
    }
}

static void Ultrasonic_Trigger(UltrasonicSensor_t sensor)
{
    UltrasonicRuntime_t *selected = &sensors[sensor];

    selected->measurementReady = 0U;
    selected->waitingForFall = 0U;

    HAL_GPIO_WritePin(selected->trigPort, selected->trigPin, GPIO_PIN_RESET);
    DelayUs(3U);
    HAL_GPIO_WritePin(selected->trigPort, selected->trigPin, GPIO_PIN_SET);
    DelayUs(10U);
    HAL_GPIO_WritePin(selected->trigPort, selected->trigPin, GPIO_PIN_RESET);
}

static uint8_t Ultrasonic_GetDistance(
    UltrasonicSensor_t sensor,
    float *distanceCm)
{
    UltrasonicRuntime_t *selected = &sensors[sensor];
    uint32_t pulseUs;
    float measuredCm;

    if (selected->measurementReady == 0U)
    {
        return 0U;
    }

    pulseUs = selected->echoPulseUs;
    selected->measurementReady = 0U;

    if (pulseUs > ULTRASONIC_TIMEOUT_US)
    {
        return 0U;
    }

    measuredCm = ((float)pulseUs * 0.0343f) / 2.0f;

    if ((measuredCm < ULTRASONIC_MIN_CM) ||
        (measuredCm > ULTRASONIC_MAX_CM))
    {
        return 0U;
    }

    *distanceCm = measuredCm;
    return 1U;
}

void Ultrasonic_Init(void)
{
    memset(&sharedData, 0, sizeof(sharedData));

    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_GPIO_WritePin(
        US_FRONT_TRIG_GPIO_Port,
        US_FRONT_TRIG_Pin,
        GPIO_PIN_RESET
    );
    HAL_GPIO_WritePin(
        US_LEFT_TRIG_GPIO_Port,
        US_LEFT_TRIG_Pin,
        GPIO_PIN_RESET
    );
    HAL_GPIO_WritePin(
        US_RIGHT_TRIG_GPIO_Port,
        US_RIGHT_TRIG_Pin,
        GPIO_PIN_RESET
    );
}

void Ultrasonic_Copy(UltrasonicData_t *destination)
{
    if (destination == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    *destination = sharedData;
    taskEXIT_CRITICAL();
}

void UltrasonicTask(void *argument)
{
    (void)argument;
    UltrasonicSensor_t sensor;

    Ultrasonic_Init();

    for (;;)
    {
        for (sensor = ULTRASONIC_FRONT;
             sensor < ULTRASONIC_COUNT;
             sensor = (UltrasonicSensor_t)(sensor + 1))
        {
            float measuredCm;

            Ultrasonic_Trigger(sensor);
            osDelay(ULTRASONIC_INTER_SENSOR_MS);

            if (Ultrasonic_GetDistance(sensor, &measuredCm) != 0U)
            {
                taskENTER_CRITICAL();

                if (sharedData.valid[sensor] == 0U)
                {
                    sharedData.distanceCm[sensor] = measuredCm;
                }
                else
                {
                    /* 75% old value and 25% new value. */
                    sharedData.distanceCm[sensor] =
                        (0.50f * sharedData.distanceCm[sensor]) +
                        (0.50f * measuredCm);
                }

                sharedData.valid[sensor] = 1U;
                sharedData.updatedMs[sensor] = HAL_GetTick();

                frontDistanceTestCm =
                    sharedData.valid[ULTRASONIC_FRONT] ?
                    sharedData.distanceCm[ULTRASONIC_FRONT] : -1.0f;
                leftDistanceTestCm =
                    sharedData.valid[ULTRASONIC_LEFT] ?
                    sharedData.distanceCm[ULTRASONIC_LEFT] : -1.0f;
                rightDistanceTestCm =
                    sharedData.valid[ULTRASONIC_RIGHT] ?
                    sharedData.distanceCm[ULTRASONIC_RIGHT] : -1.0f;

                taskEXIT_CRITICAL();
            }
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    UltrasonicRuntime_t *selected;
    uint32_t nowUs;

    if (GPIO_Pin == US_FRONT_ECHO_Pin)
    {
        selected = &sensors[ULTRASONIC_FRONT];
    }
    else if (GPIO_Pin == US_LEFT_ECHO_Pin)
    {
        selected = &sensors[ULTRASONIC_LEFT];
    }
    else if (GPIO_Pin == US_RIGHT_ECHO_Pin)
    {
        selected = &sensors[ULTRASONIC_RIGHT];
    }
    else
    {
        return;
    }

    nowUs = __HAL_TIM_GET_COUNTER(&htim2);

    if (HAL_GPIO_ReadPin(selected->echoPort, selected->echoPin) ==
        GPIO_PIN_SET)
    {
        selected->echoStartUs = nowUs;
        selected->waitingForFall = 1U;
    }
    else if (selected->waitingForFall != 0U)
    {
        selected->echoPulseUs = nowUs - selected->echoStartUs;
        selected->waitingForFall = 0U;
        selected->measurementReady = 1U;
    }
}
