#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

/* Ultrasonic timing */
#define ULTRASONIC_TIMEOUT_US             30000U
#define ULTRASONIC_INTER_SENSOR_MS           28U
#define ULTRASONIC_MIN_MM                    25U
#define ULTRASONIC_MAX_MM                  3000U
#define ULTRASONIC_STALE_MS                 450U

/* Maze-navigation thresholds */
#define FRONT_STOP_MM                       330U
#define FRONT_CLEAR_MM                      380U
#define SIDE_BLOCKED_MM                     300U
#define SIDE_OPEN_MM                        450U
#define JUNCTION_ADVANTAGE_MM               180U

/* L298N speeds, expressed as PWM percentages. */
#define DRIVE_SPEED                          32U
#define CORRECTION_SLOW_SPEED                30U
#define TURN_SPEED                           40U
#define REVERSE_SPEED                        30U

/* State timing */
#define STOP_PAUSE_MS                       300U
#define REVERSE_TIME_MS                     300U
#define TURN_MIN_TIME_MS                    320U
#define TURN_MAX_TIME_MS                   1200U
#define JUNCTION_COOLDOWN_MS               1500U

/* Button */
#define BUTTON_DEBOUNCE_MS                   50U

/* Serial module */
#define BLUETOOTH_BAUD_RATE                 9600U
#define BLUETOOTH_COMMAND_TIMEOUT_MS         600U

/* Manual mode motor settings, expressed as PWM percentages. */
#define MANUAL_DRIVE_SPEED                    70U
#define MANUAL_TURN_SPEED                     55U
#define MANUAL_CURVE_INNER_SPEED              45U
#define MANUAL_FRONT_SAFETY_MM               210U

#endif /* CAR_CONFIG_H */
