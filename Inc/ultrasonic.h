#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "cmsis_os.h"
#include <stdint.h>

typedef enum
{
    ULTRASONIC_FRONT = 0,
    ULTRASONIC_LEFT,
    ULTRASONIC_RIGHT,
    ULTRASONIC_COUNT
} UltrasonicSensor_t;

typedef struct
{
    float distanceCm[ULTRASONIC_COUNT];
    uint8_t valid[ULTRASONIC_COUNT];
    uint32_t updatedMs[ULTRASONIC_COUNT];
} UltrasonicData_t;

extern volatile float frontDistanceTestCm;
extern volatile float leftDistanceTestCm;
extern volatile float rightDistanceTestCm;

void Ultrasonic_Init(void);
void UltrasonicTask(void *argument);
void Ultrasonic_Copy(UltrasonicData_t *destination);

#endif
