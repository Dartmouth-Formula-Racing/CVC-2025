/*
 * statemachine.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <main.h>
#include <parse.h>
#include <semphr.h>
#include <statemachine.h>
#include <stdbool.h>
#include <task.h>
#include <tasks.h>
#include <throttle.h>

void StateMachine_Task(void* arguments);

static VehicleState state = WAIT_FOR_PRECHARGE;
static DriveState driveState = NEUTRAL;

static StackType_t stateMachineTaskStack[STATEMACHINE_TASK_STACK_SIZE];
static StaticTask_t stateMachineTaskTCB;

void StateMachine_Init(void) {
    TaskHandle_t handle = xTaskCreateStatic(StateMachine_Task, STATEMACHINE_TASK_NAME, STATEMACHINE_TASK_STACK_SIZE, NULL, STATEMACHINE_TASK_PRIORITY,
                                            stateMachineTaskStack, &stateMachineTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void StateMachine_Task(void* arguments) {
    bool driveLockout = true;  // Locks out drive/reverse until neutral is pressed
    TickType_t buzzerStartTime = xTaskGetTickCount();
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        TickType_t now = xTaskGetTickCount();

        if (driveLockout && HAL_GPIO_ReadPin(Neutral_Button_GPIO_Port, Neutral_Button_Pin) == GPIO_PIN_RESET) {
            driveLockout = false;
        }

        switch (state) {
            case WAIT_FOR_PRECHARGE:
                // Precharge starts when AIR 1 closes
                if (HAL_GPIO_ReadPin(MCU_Contactor_1_Closed_GPIO_Port, MCU_Contactor_1_Closed_Pin) == GPIO_PIN_SET) {
                    state = PRECHARGE;
                }
                break;
            case PRECHARGE:
                // Precharge ends when AIR 2 closes
                if (HAL_GPIO_ReadPin(MCU_Contactor_2_Closed_GPIO_Port, MCU_Contactor_2_Closed_Pin) == GPIO_PIN_SET) {
                    state = NOT_READY_TO_DRIVE;
                } else if (HAL_GPIO_ReadPin(MCU_Contactor_1_Closed_GPIO_Port, MCU_Contactor_1_Closed_Pin) == GPIO_PIN_RESET) {
                    state = WAIT_FOR_PRECHARGE;
                }
                break;
            case NOT_READY_TO_DRIVE:
                // Check if discharged
                if (HAL_GPIO_ReadPin(MCU_Contactor_1_Closed_GPIO_Port, MCU_Contactor_1_Closed_Pin) == GPIO_PIN_RESET) {
                    state = WAIT_FOR_PRECHARGE;
                    break;
                }
                if (HAL_GPIO_ReadPin(MCU_Contactor_2_Closed_GPIO_Port, MCU_Contactor_2_Closed_Pin) == GPIO_PIN_RESET) {
                    state = PRECHARGE;
                    break;
                }

                // Check if drive lockout is active
                if (driveLockout) {
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;  // Reset drive state to neutral
                    break;
                }

                // Check if throttle is valid and under threshold
                if (!Throttle_Valid() || Throttle_GetValue() > MAX_RTD_THROTTLE) {
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;  // Reset drive state to neutral
                    break;
                }

                // Check if drive button is pressed
                if (HAL_GPIO_ReadPin(Drive_Button_GPIO_Port, Drive_Button_Pin) == GPIO_PIN_RESET) {
                    // Transition to BUZZER state
                    state = BUZZER;
                    driveState = DRIVE;
                    driveLockout = true;  // Re-enable drive lockout
                    buzzerStartTime = xTaskGetTickCount();
                }

                // Check if reverse button is pressed
                if (HAL_GPIO_ReadPin(Reverse_Button_GPIO_Port, Reverse_Button_Pin) == GPIO_PIN_RESET) {
                    // Transition to BUZZER state
                    state = BUZZER;
                    if (ALLOW_REVERSE) {
                        driveState = REVERSE;
                    } else {
                        driveState = DRIVE;  // If reverse is not allowed, treat it as drive
                    }
                    driveLockout = true;  // Re-enable drive lockout
                    buzzerStartTime = xTaskGetTickCount();
                }
                break;
            case BUZZER:
                // Check if discharged
                if (HAL_GPIO_ReadPin(MCU_Contactor_1_Closed_GPIO_Port, MCU_Contactor_1_Closed_Pin) == GPIO_PIN_RESET) {
                    state = WAIT_FOR_PRECHARGE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }
                if (HAL_GPIO_ReadPin(MCU_Contactor_2_Closed_GPIO_Port, MCU_Contactor_2_Closed_Pin) == GPIO_PIN_RESET) {
                    state = PRECHARGE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }

                // Check if neutral button is pressed, neutral should be able to cancel drive/reverse
                if (HAL_GPIO_ReadPin(Neutral_Button_GPIO_Port, Neutral_Button_Pin) == GPIO_PIN_RESET) {
                    // Reset to NOT_READY_TO_DRIVE state
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }

                // Check if throttle is valid and under threshold
                if (!Throttle_Valid() || Throttle_GetValue() > MAX_RTD_THROTTLE) {
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;  // Reset drive state to neutral
                    break;
                }

                // Turn on buzzer
                if (now - buzzerStartTime >= BUZZER_TIME) {
                    state = READY_TO_DRIVE;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                } else {
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
                }
                break;
            case READY_TO_DRIVE:
                // Check if discharged
                if (HAL_GPIO_ReadPin(MCU_Contactor_1_Closed_GPIO_Port, MCU_Contactor_1_Closed_Pin) == GPIO_PIN_RESET) {
                    state = WAIT_FOR_PRECHARGE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }
                if (HAL_GPIO_ReadPin(MCU_Contactor_2_Closed_GPIO_Port, MCU_Contactor_2_Closed_Pin) == GPIO_PIN_RESET) {
                    state = PRECHARGE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }

                // Check if neutral button is pressed
                if (HAL_GPIO_ReadPin(Neutral_Button_GPIO_Port, Neutral_Button_Pin) == GPIO_PIN_RESET) {
                    // Reset to NOT_READY_TO_DRIVE state
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;
                    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
                    break;
                }

                // Check if throttle is valid
                if (!Throttle_Valid()) {
                    state = NOT_READY_TO_DRIVE;
                    driveState = NEUTRAL;  // Reset drive state to neutral
                    break;
                }
                break;
            default:
                // Invalid state, reset to WAIT_FOR_PRECHARGE
                // Should never happen
                state = WAIT_FOR_PRECHARGE;
                driveState = NEUTRAL;
                driveLockout = true;
                break;
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(STATEMACHINE_TASK_INTERVAL));
    }
}

VehicleState StateMachine_GetState(void) { return state; }
DriveState StateMachine_GetDriveState(void) { return driveState; }