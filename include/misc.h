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

#define SOFT_BRAKE_THRESHOLD 1082
#define HARD_BRAKE_THRESHOLD 1600
#define BRAKE_BLINK_INTERVAL 150

#define GEARBOX_RATIO 4.2
#define WHEEL_DIAMETER 18                                               // inches
#define EFFICIENCY_PERIOD 100                                           // ms
#define EFFICIENCY_WINDOW 30                                            // Seconds over which to measure efficiency
#define EFFICIENCY_LENGTH EFFICIENCY_WINDOW * 1000 / EFFICIENCY_PERIOD  // Number of samples to average over
#define EFFICIENCY_MAX 2000                                             // Maximum efficiency value (W/km)

#define MAX_IDLE_COUNT 2000  // TODO: Calibrate

// Dashboard / telemetry broadcast on BUS1. The task wakes every DASHBOARD_BROADCAST_TASK_INTERVAL (10 ms);
// each frame is sent every N wake-ups (N * 10 ms). This used to be all six frames every 10 ms = 600 frames/s,
// which is now ~90 frames/s. The PHASE values stagger the frames so they never burst together.
#define DASH_PERIOD_STATE 5      // 0x750 vehicle state / safety / RTD failure -> 20 Hz
#define DASH_PERIOD_INPUTS 2     // 0x755 throttle / steering / brake         -> 50 Hz (pedal trace for logs)
#define DASH_PERIOD_BATTERY 10   // 0x752 pack V / I / SOC / SOH              -> 10 Hz (EMUS itself only updates at 1 Hz)
#define DASH_PERIOD_AIR 20       // 0x754 AIR status                          -> 5 Hz
#define DASH_PERIOD_TEMPS 50     // 0x753 motor / inverter temperatures       -> 2 Hz
#define DASH_PERIOD_DRIVING 100  // 0x751 avg rpm / efficiency / odometer     -> 1 Hz (placeholder zeros for now)

#define DASH_PHASE_STATE 0
#define DASH_PHASE_INPUTS 1
#define DASH_PHASE_BATTERY 2
#define DASH_PHASE_AIR 3
#define DASH_PHASE_TEMPS 4
#define DASH_PHASE_DRIVING 5

typedef enum {
    RELEASED,
    SOFT_BRAKE,
    HARD_BRAKE,
} Brake_State;

void Misc_Init(void);
Brake_State Brake_GetState(void);

#endif /* MISC_H */