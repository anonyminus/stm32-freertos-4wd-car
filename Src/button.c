
#include "button.h"
#include "car_config.h"
#include "cmsis_os.h"
#include "main.h"
#include "mode.h"

void ButtonTask(void *argument)
{
    (void)argument;

    GPIO_PinState previousState;

    previousState = HAL_GPIO_ReadPin(
        AUTO_BTN_GPIO_Port,
        AUTO_BTN_Pin
    );

    HAL_GPIO_WritePin(
        RTOS_LED_GPIO_Port,
        RTOS_LED_Pin,
        GPIO_PIN_RESET
    );

    for (;;)
    {
        GPIO_PinState currentState;

        currentState = HAL_GPIO_ReadPin(
            AUTO_BTN_GPIO_Port,
            AUTO_BTN_Pin
        );

        /* Detect button press: HIGH to LOW. */
        if ((previousState == GPIO_PIN_SET) &&
            (currentState == GPIO_PIN_RESET))
        {
            osDelay(BUTTON_DEBOUNCE_MS);

            if (HAL_GPIO_ReadPin(
                    AUTO_BTN_GPIO_Port,
                    AUTO_BTN_Pin) == GPIO_PIN_RESET)
            {
                if (Mode_Get() == MODE_AUTO)
                {
                    Mode_Set(MODE_MANUAL);

                    HAL_GPIO_WritePin(
                        RTOS_LED_GPIO_Port,
                        RTOS_LED_Pin,
                        GPIO_PIN_RESET
                    );
                }
                else
                {
                    Mode_Set(MODE_AUTO);

                    HAL_GPIO_WritePin(
                        RTOS_LED_GPIO_Port,
                        RTOS_LED_Pin,
                        GPIO_PIN_SET
                    );
                }

                /* Wait for button release. */
                while (HAL_GPIO_ReadPin(
                           AUTO_BTN_GPIO_Port,
                           AUTO_BTN_Pin) == GPIO_PIN_RESET)
                {
                    osDelay(10U);
                }

                currentState = GPIO_PIN_SET;
            }
        }

        previousState = currentState;
        osDelay(10U);
    }
}
