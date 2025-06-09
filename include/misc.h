/*
 * misc.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef MISC_H
#define MISC_H

#define INVERTER_TEMP_MIN 15
#define INVERTER_TEMP_MAX 30
#define MOTOR_TEMP_MIN 20
#define MOTOR_TEMP_MAX 40

#define PUMP_MIN_DC 0.7
#define PUMP_MAX_DC 1.0

#define SOFT_BRAKE_THRESHOLD 0.90
#define HARD_BRAKE_THRESHOLD 1.40
#define BRAKE_BLINK_INTERVAL 150

#define GEARBOX_RATIO 4.2
#define WHEEL_DIAMETER 18                                               // inches
#define EFFICIENCY_PERIOD 100                                           // ms
#define EFFICIENCY_WINDOW 30                                            // Seconds over which to measure efficiency
#define EFFICIENCY_LENGTH EFFICIENCY_WINDOW * 1000 / EFFICIENCY_PERIOD  // Number of samples to average over
#define EFFICIENCY_MAX 2000                                             // Maximum efficiency value (W/km)

#define MAX_IDLE_COUNT 2000  // TODO: Calibrate

typedef enum {
    RELEASED,
    SOFT_BRAKE,
    HARD_BRAKE,
} Brake_State;

#define BRAKE_SOFT_THRESHOLD 1082
#define BRAKE_HARD_THRESHOLD 1600

void Misc_Init(void);
Brake_State Brake_GetState(void);

#endif /* MISC_H */