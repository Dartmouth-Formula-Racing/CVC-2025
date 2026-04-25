/*
 * misc.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <analogs.h>
#include <can.h>
#include <data.h>
#include <main.h>
#include <misc.h>
#include <parse.h>
#include <statemachine.h>
#include <task.h>
#include <tasks.h>
#include <throttle.h>

static StaticTask_t stateMachineOutputTaskTCB;
static StackType_t stateMachineOutputTaskStack[STATEMACHINE_OUTPUT_TASK_STACK_SIZE];
static StaticTask_t BrakeTaskTCB;
static StackType_t BrakeTaskStack[BRAKE_LIGHT_TASK_STACK_SIZE];
static StaticTask_t DashboardBroadcastTaskTCB;
static StackType_t DashboardBroadcastTaskStack[DASHBOARD_BROADCAST_TASK_STACK_SIZE];

static Brake_State brakeState = RELEASED;

extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim12;

void StateMachine_Output_Task(void* arguments);
void Brake_Task(void* arguments);
void Dashboard_Broadcast_Task(void* arguments);

void Misc_Init(void) {
    TaskHandle_t handle = xTaskCreateStatic(StateMachine_Output_Task, STATEMACHINE_OUTPUT_TASK_NAME, STATEMACHINE_OUTPUT_TASK_STACK_SIZE, NULL,
                                            STATEMACHINE_OUTPUT_TASK_PRIORITY, stateMachineOutputTaskStack, &stateMachineOutputTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
    handle = xTaskCreateStatic(Brake_Task, BRAKE_LIGHT_TASK_NAME, BRAKE_LIGHT_TASK_STACK_SIZE, NULL, BRAKE_LIGHT_TASK_PRIORITY, BrakeTaskStack, &BrakeTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
    handle = xTaskCreateStatic(Dashboard_Broadcast_Task, DASHBOARD_BROADCAST_TASK_NAME, DASHBOARD_BROADCAST_TASK_STACK_SIZE, NULL,
                               DASHBOARD_BROADCAST_TASK_PRIORITY, DashboardBroadcastTaskStack, &DashboardBroadcastTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1) != HAL_OK || HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
}

void StateMachine_Output_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        if (StateMachine_GetState() == BUZZER || StateMachine_GetState() == READY_TO_DRIVE) {
            HAL_GPIO_WritePin(Left_Inverter_Enable_GPIO_Port, Left_Inverter_Enable_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Right_Inverter_Enable_GPIO_Port, Right_Inverter_Enable_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(Left_Inverter_Enable_GPIO_Port, Left_Inverter_Enable_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Right_Inverter_Enable_GPIO_Port, Right_Inverter_Enable_Pin, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, 0);
        }

        if (StateMachine_GetDriveState() != NEUTRAL) {
            CAN_Parse_Inverter_Temp1(RL);
            CAN_Parse_Inverter_Temp1(RR);
            CAN_Parse_Inverter_Temp3TorqueShudder(RL);
            CAN_Parse_Inverter_Temp3TorqueShudder(RR);

            float tempA = (float)InverterRLDataGet(INVERTER_POWER_MODULE_A_TEMP).data / 10.0f;
            float tempB = (float)InverterRLDataGet(INVERTER_POWER_MODULE_B_TEMP).data / 10.0f;
            float tempC = (float)InverterRLDataGet(INVERTER_POWER_MODULE_C_TEMP).data / 10.0f;
            float motorTemp = (float)InverterRLDataGet(INVERTER_MOTOR_TEMP).data / 10.0f;

            float maxTemp = tempA;
            maxTemp = (tempB > maxTemp) ? tempB : maxTemp;
            maxTemp = (tempC > maxTemp) ? tempC : maxTemp;

            float inverterDutyCycle = 0.0f;
            if (maxTemp < INVERTER_TEMP_MIN) {
                inverterDutyCycle = PUMP_MIN_DC;
            } else if (maxTemp > INVERTER_TEMP_MAX) {
                inverterDutyCycle = PUMP_MAX_DC;
            } else {
                inverterDutyCycle = PUMP_MIN_DC + (maxTemp - INVERTER_TEMP_MIN) / (INVERTER_TEMP_MAX - INVERTER_TEMP_MIN) * (PUMP_MAX_DC - PUMP_MIN_DC);
            }

            float motorDutyCycle = 0.0f;

            if (motorTemp < MOTOR_TEMP_MIN) {
                motorDutyCycle = PUMP_MIN_DC;
            } else if (motorTemp > MOTOR_TEMP_MAX) {
                motorDutyCycle = PUMP_MAX_DC;
            } else {
                motorDutyCycle = PUMP_MIN_DC + (motorTemp - MOTOR_TEMP_MIN) / (MOTOR_TEMP_MAX - MOTOR_TEMP_MIN) * (PUMP_MAX_DC - PUMP_MIN_DC);
            }

            float pumpDutyCycle = inverterDutyCycle > motorDutyCycle ? inverterDutyCycle : motorDutyCycle;
            __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, (uint32_t)(htim11.Init.Period * pumpDutyCycle));

            tempA = (float)InverterRRDataGet(INVERTER_POWER_MODULE_A_TEMP).data / 10.0f;
            tempB = (float)InverterRRDataGet(INVERTER_POWER_MODULE_B_TEMP).data / 10.0f;
            tempC = (float)InverterRRDataGet(INVERTER_POWER_MODULE_C_TEMP).data / 10.0f;
            motorTemp = (float)InverterRRDataGet(INVERTER_MOTOR_TEMP).data / 10.0f;
            maxTemp = tempA;
            maxTemp = (tempB > maxTemp) ? tempB : maxTemp;
            maxTemp = (tempC > maxTemp) ? tempC : maxTemp;

            if (maxTemp < INVERTER_TEMP_MIN) {
                inverterDutyCycle = PUMP_MIN_DC;
            } else if (maxTemp > INVERTER_TEMP_MAX) {
                inverterDutyCycle = PUMP_MAX_DC;
            } else {
                inverterDutyCycle = PUMP_MIN_DC + (maxTemp - INVERTER_TEMP_MIN) / (INVERTER_TEMP_MAX - INVERTER_TEMP_MIN) * (PUMP_MAX_DC - PUMP_MIN_DC);
            }

            if (motorTemp < MOTOR_TEMP_MIN) {
                motorDutyCycle = PUMP_MIN_DC;
            } else if (motorTemp > MOTOR_TEMP_MAX) {
                motorDutyCycle = PUMP_MAX_DC;
            } else {
                motorDutyCycle = PUMP_MIN_DC + (motorTemp - MOTOR_TEMP_MIN) / (MOTOR_TEMP_MAX - MOTOR_TEMP_MIN) * (PUMP_MAX_DC - PUMP_MIN_DC);
            }

            pumpDutyCycle = inverterDutyCycle > motorDutyCycle ? inverterDutyCycle : motorDutyCycle;
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, (uint32_t)(htim12.Init.Period * pumpDutyCycle));
        } else {
            __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, 0);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(STATEMACHINE_OUTPUT_TASK_INTERVAL));
    }
}

void Brake_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t lightBlinkTime = xTaskGetTickCount();
    GPIO_PinState lightState = GPIO_PIN_RESET;

    while (1) {
        if (xTaskGetTickCount() - lightBlinkTime >= pdMS_TO_TICKS(BRAKE_BLINK_INTERVAL)) {
            lightBlinkTime = xTaskGetTickCount();
            lightState = (lightState == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
        }
        float brakeValue = Analogs_ReadChannel(Brake_Pressure);
        if (brakeValue >= HARD_BRAKE_THRESHOLD && brakeValue != ANALOG_INVALID) {
            brakeState = HARD_BRAKE;
            HAL_GPIO_WritePin(Brake_Light_GPIO_Port, Brake_Light_Pin, lightState);
        } else if (brakeValue >= SOFT_BRAKE_THRESHOLD && brakeValue != ANALOG_INVALID) {
            brakeState = SOFT_BRAKE;
            HAL_GPIO_WritePin(Brake_Light_GPIO_Port, Brake_Light_Pin, GPIO_PIN_SET);
        } else {
            brakeState = RELEASED;
            HAL_GPIO_WritePin(Brake_Light_GPIO_Port, Brake_Light_Pin, GPIO_PIN_RESET);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(BRAKE_LIGHT_TASK_INTERVAL));
    }
}

Brake_State Brake_GetState(void) { return brakeState; }

void Dashboard_Broadcast_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        CAN_Frame frame = {0};
        frame.header.tx.StdId = Dashboard_STD(0);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;
        frame.data[0] = HAL_GPIO_ReadPin(MCU_BMS_OK_GPIO_Port, MCU_BMS_OK_Pin) == GPIO_PIN_SET;
        frame.data[1] = HAL_GPIO_ReadPin(MCU_IMD_OK_GPIO_Port, MCU_IMD_OK_Pin) == GPIO_PIN_SET;
        frame.data[2] = HAL_GPIO_ReadPin(MCU_BSPD_OK_GPIO_Port, MCU_BSPD_OK_Pin) == GPIO_PIN_SET;
        frame.data[3] = HAL_GPIO_ReadPin(MCU_BSPD_Instant_GPIO_Port, MCU_BSPD_Instant_Pin) == GPIO_PIN_SET;
        frame.data[4] = (uint8_t)StateMachine_GetDriveState();
        frame.data[5] = (uint8_t)StateMachine_GetState();
        frame.data[6] = 0;
        frame.data[7] = 0;
        CAN_SendFrame(BUS1, &frame);

        frame.header.tx.StdId = Dashboard_STD(1);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        int16_t avg_rpm = 0;     // TODO: Implement wheel speed sensing
        int16_t efficiency = 0;  // TODO: Implement efficiency calculation
        uint16_t odometer = 0;   // TODO: Implement odometer calculation

        frame.data[0] = ((uint16_t)(Throttle_GetValue() * 1000.0f) >> 8) & 0xFF;
        frame.data[1] = ((uint16_t)(Throttle_GetValue() * 1000.0f) & 0xFF);
        frame.data[2] = (avg_rpm >> 8) & 0xFF;
        frame.data[3] = avg_rpm & 0xFF;
        frame.data[4] = (efficiency >> 8) & 0xFF;
        frame.data[5] = efficiency & 0xFF;
        frame.data[6] = (odometer >> 8) & 0xFF;
        frame.data[7] = odometer & 0xFF;
        CAN_SendFrame(BUS1, &frame);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DASHBOARD_BROADCAST_TASK_INTERVAL));
    }
}