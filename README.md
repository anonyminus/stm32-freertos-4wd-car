# STM32 FreeRTOS 4WD Autonomous & Wi-Fi RC Car

## Overview

This project is a four-wheel-drive car built with the STM32 NUCLEO-F411RE. It supports both manual Wi-Fi remote control and autonomous driving.

The ESP-01S creates a Wi-Fi access point and hosts a mobile-friendly control webpage. In autonomous mode, three ultrasonic sensors measure the front, left, and right distances so the car can detect obstacles and select a safe direction.

FreeRTOS manages the manual control, Wi-Fi communication, ultrasonic measurement, autonomous driving, and mode-control tasks.

## Demonstration

[▶ Watch the car demonstration] https://youtube.com/shorts/UZM03QLA61s?si=9ukpCIgxdn7mIF0P

## Main Features

* Manual and autonomous operating modes
* Web-based Wi-Fi remote control
* Manual forward, backward, left, right, and stop controls
* Adjustable manual driving speed
* Physical and webpage mode controls
* Three-direction ultrasonic distance measurement
* Obstacle detection and avoidance
* Automatic path-centering correction
* FreeRTOS-based multitasking
* Safe motor stop during mode changes
* Command timeout for communication safety

## Hardware

| Component                | Purpose                                     |
| ------------------------ | ------------------------------------------- |
| NUCLEO-F411RE            | Main controller                             |
| ESP-01S                  | Wi-Fi access point and web server           |
| L298N                    | Dual DC motor driver                        |
| Four DC motors           | Four-wheel-drive movement                   |
| Three ultrasonic sensors | Front, left, and right distance measurement |
| 4WD chassis              | Car platform                                |
| 12 V battery pack        | Motor and system power                      |
| User button              | Physical mode switching                     |
| Onboard LED              | Current-mode indication                     |

## Software

* STM32CubeIDE
* STM32CubeMX
* Arduino IDE
* FreeRTOS
* STM32 HAL
* ESP8266 Wi-Fi and WebServer libraries

## Operating Modes

### Manual Mode

The car is controlled through a webpage hosted by the ESP-01S.

Available commands:

| Command | Action                    |
| ------- | ------------------------- |
| `F`     | Move forward              |
| `B`     | Move backward             |
| `L`     | Pivot left                |
| `R`     | Pivot right               |
| `S`     | Stop                      |
| `+`     | Increase speed            |
| `-`     | Decrease speed            |
| `A`     | Switch to autonomous mode |
| `M`     | Switch to manual mode     |

The car always starts in manual mode with the motors stopped.

### Autonomous Mode

The car uses front, left, and right ultrasonic sensors to examine its surroundings.

The autonomous controller can:

* Drive forward when the path is clear
* Correct its direction to remain near the center of the track
* Stop when an obstacle is detected
* Compare the left and right distances
* Select the direction with more available space
* Reverse briefly when both sides are blocked
* Perform pivot turns at sharp corners

The motor speeds, obstacle thresholds, and turning times were tuned for the test track.

## Wi-Fi Control

The ESP-01S operates as a Wi-Fi access point.

```text
Network name: STM32_RC_CAR
IP address:   192.168.4.1
```

After connecting to the network, the user opens `192.168.4.1` in a browser. The webpage provides direction, speed, stop, and mode controls.

Commands are sent from the ESP-01S to the STM32 through USART1 at 115200 baud.

Each command uses this frame format:

```text
<command>
```

Examples:

```text
<F>
<S>
<A>
<M>
```

## FreeRTOS Tasks

| Task              | Responsibility                               |
| ----------------- | -------------------------------------------- |
| `WifiTask`        | Processes commands received from the ESP-01S |
| `ManualDriveTask` | Controls the motors in manual mode           |
| `AutoDriveTask`   | Executes autonomous navigation               |
| `UltrasonicTask`  | Measures front, left, and right distances    |
| `ButtonTask`      | Handles the physical mode button             |
| `defaultTask`     | Reserved background task                     |

Only the task associated with the active mode controls the motors.

## Mode Switching

The operating mode can be changed using:

* The mode button on the control webpage
* The physical user button on the Nucleo board

Every mode change stops the motors before transferring control to the other driving task.

The onboard LED indicates the STM32 mode:

| LED | Mode       |
| --- | ---------- |
| Off | Manual     |
| On  | Autonomous |

## Project Structure

```text
Core/
├── Inc/
│   ├── auto_drive.h
│   ├── button.h
│   ├── car_config.h
│   ├── manual_drive.h
│   ├── mode.h
│   ├── motor.h
│   ├── pwm.h
│   ├── ultrasonic.h
│   └── wifi.h
└── Src/
    ├── auto_drive.c
    ├── button.c
    ├── freertos.c
    ├── main.c
    ├── manual_drive.c
    ├── mode.c
    ├── motor.c
    ├── pwm.c
    ├── ultrasonic.c
    └── wifi.c
```

## System Flow

```text
Web controller
      ↓
ESP-01S
      ↓ USART1
STM32 Wi-Fi task
      ↓
Mode controller
  ┌───┴────────┐
  ↓            ↓
Manual task    Autonomous task
  └──────┬─────┘
         ↓
     Motor driver
         ↓
      4WD motors
```

## Skills Demonstrated

* STM32 peripheral configuration
* FreeRTOS task design
* UART interrupt communication
* PWM motor-speed control
* Web-based embedded control
* Ultrasonic sensor interfacing
* State-machine design
* Autonomous navigation logic
* Real-time debugging and track tuning
* Embedded hardware integration

## Author

GitHub: @anonyminus

Embedded systems project developed using the STM32 NUCLEO-F411RE, FreeRTOS, and ESP8266.

