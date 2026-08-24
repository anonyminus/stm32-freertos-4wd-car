

#include "wifi.h"
#include "main.h"
#include "mode.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

static volatile uint8_t uartRxByte;
static volatile uint8_t pendingCommand;
static volatile uint8_t commandReady;

static volatile uint8_t frameState;
static volatile uint8_t frameCommand;

static WifiManualState_t manualState = {
    MANUAL_CMD_STOP,
    0U,
    0U
};

static uint8_t Wifi_IsValidCharacter(uint8_t command)
{
    return (command == 'F') ||
           (command == 'B') ||
           (command == 'L') ||
           (command == 'R') ||
           (command == 'S') ||
           (command == '+') ||
           (command == '-') ||
           (command == 'A') ||
           (command == 'M');
}

static ManualCommand_t Wifi_DecodeManualCommand(uint8_t command)
{
    switch (command)
    {
        case 'F': return MANUAL_CMD_FORWARD;
        case 'B': return MANUAL_CMD_BACKWARD;
        case 'L': return MANUAL_CMD_LEFT;
        case 'R': return MANUAL_CMD_RIGHT;
        case '+': return MANUAL_CMD_SPEED_UP;
        case '-': return MANUAL_CMD_SPEED_DOWN;
        case 'S':
        default:  return MANUAL_CMD_STOP;
    }
}

static void Wifi_StoreManualCommand(ManualCommand_t command)
{
    taskENTER_CRITICAL();
    manualState.command = command;
    manualState.updatedMs = HAL_GetTick();
    manualState.sequence++;
    taskEXIT_CRITICAL();
}

void Wifi_Init(void)
{
    frameState = 0U;
    commandReady = 0U;
    manualState.updatedMs = HAL_GetTick();

    if (HAL_UART_Receive_IT(
            &huart1,
            (uint8_t *)&uartRxByte,
            1U) != HAL_OK)
    {
        Error_Handler();
    }
}

void Wifi_GetManualState(WifiManualState_t *destination)
{
    if (destination == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    *destination = manualState;
    taskEXIT_CRITICAL();
}

void WifiTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        uint8_t command = 0U;
        uint8_t hasCommand = 0U;

        taskENTER_CRITICAL();
        if (commandReady != 0U)
        {
            command = pendingCommand;
            commandReady = 0U;
            hasCommand = 1U;
        }
        taskEXIT_CRITICAL();

        if (hasCommand != 0U)
        {
            if (command == 'A')
            {
                Wifi_StoreManualCommand(MANUAL_CMD_STOP);
                Mode_Set(MODE_AUTO);
            }
            else if (command == 'M')
            {
                Mode_Set(MODE_MANUAL);
                Wifi_StoreManualCommand(MANUAL_CMD_STOP);
            }
            else if (Mode_Get() == MODE_MANUAL)
            {
                Wifi_StoreManualCommand(
                    Wifi_DecodeManualCommand(command)
                );
            }
        }

        osDelay(10U);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1)
    {
        return;
    }

    if (frameState == 0U)
    {
        if (uartRxByte == '<')
        {
            frameState = 1U;
        }
    }
    else if (frameState == 1U)
    {
        if (Wifi_IsValidCharacter(uartRxByte) != 0U)
        {
            frameCommand = uartRxByte;
            frameState = 2U;
        }
        else
        {
            frameState = 0U;
        }
    }
    else
    {
        if (uartRxByte == '>')
        {
            pendingCommand = frameCommand;
            commandReady = 1U;
        }

        frameState = 0U;
    }

    if (HAL_UART_Receive_IT(
            &huart1,
            (uint8_t *)&uartRxByte,
            1U) != HAL_OK)
    {
        Error_Handler();
    }
}
