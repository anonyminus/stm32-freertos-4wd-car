#ifndef WIFI_H
#define WIFI_H

#include "cmsis_os.h"
#include <stdint.h>

typedef enum
{
    MANUAL_CMD_STOP = 0,
    MANUAL_CMD_FORWARD,
    MANUAL_CMD_BACKWARD,
    MANUAL_CMD_LEFT,
    MANUAL_CMD_RIGHT,
    MANUAL_CMD_SPEED_UP,
    MANUAL_CMD_SPEED_DOWN
} ManualCommand_t;

typedef struct
{
    ManualCommand_t command;
    uint32_t updatedMs;
    uint32_t sequence;
} WifiManualState_t;

void Wifi_Init(void);
void WifiTask(void *argument);
void Wifi_GetManualState(WifiManualState_t *destination);

#endif
