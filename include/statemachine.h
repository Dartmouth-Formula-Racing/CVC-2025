/*
 * statemachine.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#define BUZZER_TIME 1000       // EV.9.6 (EV9.2), RTD sound must be 1-3 seconds long
#define MAX_RTD_THROTTLE 0.05  // Only enter ready to drive state if throttle is below this value
#define ALLOW_REVERSE 1        // FSAE EV.12.1.7 Vehicles must not be driven in reverse, FH&E has no such rule

typedef enum {
    WAIT_FOR_PRECHARGE,
    PRECHARGE,
    NOT_READY_TO_DRIVE,
    BUZZER,
    READY_TO_DRIVE,
} VehicleState;

typedef enum {
    DRIVE,
    NEUTRAL,
    REVERSE,
} DriveState;

void StateMachine_Init(void);
VehicleState StateMachine_GetState(void);
DriveState StateMachine_GetDriveState(void);

#endif /* STATEMACHINE_H */