
#include "mode.h"
#include "motor.h"

static volatile CarMode_t currentMode = MODE_MANUAL;

void Mode_Init(void)
{
    currentMode = MODE_MANUAL;
    Motor_Stop();
}

void Mode_Set(CarMode_t newMode)
{
    if ((newMode != MODE_MANUAL) && (newMode != MODE_AUTO))
    {
        return;
    }

    if (newMode != currentMode)
    {
        /* Stop before motor ownership passes to the other mode. */
        Motor_Stop();
        currentMode = newMode;
    }
}

CarMode_t Mode_Get(void)
{
    return currentMode;
}


