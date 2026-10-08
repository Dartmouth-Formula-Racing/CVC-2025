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

// static StaticTask_t stateMachineOutputTaskTCB;
// static StackType_t stateMachineOutputTaskStack[STATEMACHINE_OUTPUT_TASK_STACK_SIZE];
static StaticTask_t BrakeTaskTCB;
static StackType_t BrakeTaskStack[BRAKE_LIGHT_TASK_STACK_SIZE];
static StaticTask_t DashboardBroadcastTaskTCB;
static StackType_t DashboardBroadcastTaskStack[DASHBOARD_BROADCAST_TASK_STACK_SIZE];

static Brake_State brakeState = RELEASED;

extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim12;

// void StateMachine_Output_Task(void* arguments);
void Brake_Task(void* arguments);
void Dashboard_Broadcast_Task(void* arguments);

void Misc_Init(void) {
    // TaskHandle_t handle = xTaskCreateStatic(StateMachine_Output_Task, STATEMACHINE_OUTPUT_TASK_NAME, STATEMACHINE_OUTPUT_TASK_STACK_SIZE, NULL,
    //                                         STATEMACHINE_OUTPUT_TASK_PRIORITY, stateMachineOutputTaskStack, &stateMachineOutputTaskTCB);
    // if (handle == NULL) {
    //     Error_Handler();
    // }
    TaskHandle_t handle;
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

/*
void StateMachine_Output_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        if (StateMachine_GetState() == BUZZER || StateMachine_GetState() == READY_TO_DRIVE) {
            HAL_GPIO_WritePin(Left_Inverter_Enable_GPIO_Port, Left_Inverter_Enable_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Right_Inverter_Enable_GPIO_Port, Right_Inverter_Enable_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(Left_Inverter_Enable_GPIO_Port, Left_Inverter_Enable_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Right_Inverter_Enable_GPIO_Port, Right_Inverter_Enable_Pin, GPIO_PIN_RESET);
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
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, htim12.Init.Period);
        } else {
            __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, 0);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(STATEMACHINE_OUTPUT_TASK_INTERVAL));
    }
}
*/

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

void Dashboard_Broadcast_Task(void* arguments)
{
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1)
    {
        CAN_Frame frame = {0};

        CAN_Parse_EMUS_BatteryVoltageOverallParameters();
        CAN_Parse_EMUS_StateOfChargeParameters();

        CAN_Parse_Inverter_Temp1(RL);
        CAN_Parse_Inverter_Temp1(RR);

        CAN_Parse_Inverter_Temp3TorqueShudder(RL);
        CAN_Parse_Inverter_Temp3TorqueShudder(RR);


        // ---------------------------------------------------------
        // 0x750 - Vehicle state / safety / RTD failure
        // ---------------------------------------------------------

        frame.header.tx.StdId = Dashboard_STD(0);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        frame.data[0] =
            HAL_GPIO_ReadPin(MCU_BMS_OK_GPIO_Port,
                             MCU_BMS_OK_Pin) == GPIO_PIN_SET;

        frame.data[1] =
            HAL_GPIO_ReadPin(MCU_IMD_OK_GPIO_Port,
                             MCU_IMD_OK_Pin) == GPIO_PIN_SET;

        frame.data[2] =
            HAL_GPIO_ReadPin(MCU_BSPD_OK_GPIO_Port,
                             MCU_BSPD_OK_Pin) == GPIO_PIN_SET;

        frame.data[3] =
            HAL_GPIO_ReadPin(MCU_BSPD_Instant_GPIO_Port,
                             MCU_BSPD_Instant_Pin) == GPIO_PIN_SET;

        frame.data[4] = (uint8_t)StateMachine_GetDriveState();
        frame.data[5] = (uint8_t)StateMachine_GetState();
        frame.data[6] = (uint8_t)StateMachine_GetLastFailure();
        frame.data[7] = 0;

        CAN_SendFrame(BUS1, &frame);


        // ---------------------------------------------------------
        // 0x751 - Driving data
        //
        // 0-1: average motor speed, rpm
        // 2-3: efficiency, W/km
        // 4-5: odometer, m
        // 6-7: reserved
        // ---------------------------------------------------------

        frame = (CAN_Frame){0};

        frame.header.tx.StdId = Dashboard_STD(1);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        int16_t avg_rpm = 0;
        int16_t efficiency = 0;
        uint16_t odometer = 0;

        frame.data[0] = (avg_rpm >> 8) & 0xFF;
        frame.data[1] = avg_rpm & 0xFF;

        frame.data[2] = (efficiency >> 8) & 0xFF;
        frame.data[3] = efficiency & 0xFF;

        frame.data[4] = (odometer >> 8) & 0xFF;
        frame.data[5] = odometer & 0xFF;

        frame.data[6] = 0;
        frame.data[7] = 0;

        CAN_SendFrame(BUS1, &frame);


        // ---------------------------------------------------------
        // 0x752 - Battery
        //
        // 0-1: pack voltage, 0.01 V
        // 2-3: pack current, signed 0.1 A
        // 4:   SOC, %
        // 5:   SOH, %
        // 6-7: reserved
        // ---------------------------------------------------------

        frame = (CAN_Frame){0};

        frame.header.tx.StdId = Dashboard_STD(2);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        uint32_t packVoltageRaw =
            BMSDataGet(BMS_TOTAL_VOLTAGE).data;

        int16_t packCurrentRaw =
            (int16_t)BMSDataGet(BMS_CURRENT).data;

        uint8_t soc =
            (uint8_t)BMSDataGet(BMS_ESTIMATED_SOC).data;

        uint8_t soh =
            (uint8_t)BMSDataGet(BMS_ESTIMATED_SOH).data;

        uint16_t packVoltage = (uint16_t)packVoltageRaw;

        frame.data[0] = (packVoltage >> 8) & 0xFF;
        frame.data[1] = packVoltage & 0xFF;

        frame.data[2] = ((uint16_t)packCurrentRaw >> 8) & 0xFF;
        frame.data[3] = (uint16_t)packCurrentRaw & 0xFF;

        frame.data[4] = soc;
        frame.data[5] = soh;
        frame.data[6] = 0;
        frame.data[7] = 0;

        CAN_SendFrame(BUS1, &frame);


        // ---------------------------------------------------------
        // 0x753 - Powertrain temperatures
        //
        // All temperatures are signed, 0.1 C
        //
        // 0-1: left motor
        // 2-3: left inverter max module temp
        // 4-5: right motor
        // 6-7: right inverter max module temp
        // ---------------------------------------------------------

        frame = (CAN_Frame){0};

        frame.header.tx.StdId = Dashboard_STD(3);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        int16_t motorLeft =
            (int16_t)InverterRLDataGet(INVERTER_MOTOR_TEMP).data;

        int16_t motorRight =
            (int16_t)InverterRRDataGet(INVERTER_MOTOR_TEMP).data;


        int16_t invLeftA =
            (int16_t)InverterRLDataGet(INVERTER_POWER_MODULE_A_TEMP).data;

        int16_t invLeftB =
            (int16_t)InverterRLDataGet(INVERTER_POWER_MODULE_B_TEMP).data;

        int16_t invLeftC =
            (int16_t)InverterRLDataGet(INVERTER_POWER_MODULE_C_TEMP).data;


        int16_t invRightA =
            (int16_t)InverterRRDataGet(INVERTER_POWER_MODULE_A_TEMP).data;

        int16_t invRightB =
            (int16_t)InverterRRDataGet(INVERTER_POWER_MODULE_B_TEMP).data;

        int16_t invRightC =
            (int16_t)InverterRRDataGet(INVERTER_POWER_MODULE_C_TEMP).data;


        int16_t inverterLeft = invLeftA;

        if (invLeftB > inverterLeft)
            inverterLeft = invLeftB;

        if (invLeftC > inverterLeft)
            inverterLeft = invLeftC;


        int16_t inverterRight = invRightA;

        if (invRightB > inverterRight)
            inverterRight = invRightB;

        if (invRightC > inverterRight)
            inverterRight = invRightC;


        frame.data[0] = ((uint16_t)motorLeft >> 8) & 0xFF;
        frame.data[1] = (uint16_t)motorLeft & 0xFF;

        frame.data[2] = ((uint16_t)inverterLeft >> 8) & 0xFF;
        frame.data[3] = (uint16_t)inverterLeft & 0xFF;

        frame.data[4] = ((uint16_t)motorRight >> 8) & 0xFF;
        frame.data[5] = (uint16_t)motorRight & 0xFF;

        frame.data[6] = ((uint16_t)inverterRight >> 8) & 0xFF;
        frame.data[7] = (uint16_t)inverterRight & 0xFF;

        CAN_SendFrame(BUS1, &frame);


        // ---------------------------------------------------------
        // 0x754 - AIR status
        //
        // 0 = open, 1 = closed
        // 0: AIR1 closed status
        // 1: AIR2 closed status
        // 2-7: reserved
        // ---------------------------------------------------------

        frame = (CAN_Frame){0};

        frame.header.tx.StdId = Dashboard_STD(4);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        frame.data[0] = (uint8_t)CVCDataGet(CVC_AIR_1_STATE).data;
        frame.data[1] = (uint8_t)CVCDataGet(CVC_AIR_2_STATE).data;
        frame.data[2] = 0;
        frame.data[3] = 0;
        frame.data[4] = 0;
        frame.data[5] = 0;
        frame.data[6] = 0;
        frame.data[7] = 0;

        CAN_SendFrame(BUS1, &frame);


        // ---------------------------------------------------------
        // 0x755 - Driver inputs
        //
        // 0-1: throttle, 0.001 (0-1000)
        // 2-3: steering angle, raw ADC counts
        // 4-5: brake pressure, raw ADC counts
        // 6-7: reserved
        // ---------------------------------------------------------

        frame = (CAN_Frame){0};

        frame.header.tx.StdId = Dashboard_STD(5);
        frame.header.tx.IDE = CAN_ID_STD;
        frame.header.tx.RTR = CAN_RTR_DATA;
        frame.header.tx.DLC = 8;
        frame.header.tx.TransmitGlobalTime = DISABLE;

        uint16_t throttle =
            (uint16_t)(Throttle_GetValue() * 1000.0f);
        uint16_t steeringAngleRaw = Analogs_ReadChannel(Steering_Angle);
        uint16_t brakePressureRaw = Analogs_ReadChannel(Brake_Pressure);

        frame.data[0] = (throttle >> 8) & 0xFF;
        frame.data[1] = throttle & 0xFF;
        frame.data[2] = (steeringAngleRaw >> 8) & 0xFF;
        frame.data[3] = steeringAngleRaw & 0xFF;
        frame.data[4] = (brakePressureRaw >> 8) & 0xFF;
        frame.data[5] = brakePressureRaw & 0xFF;
        frame.data[6] = 0;
        frame.data[7] = 0;

        CAN_SendFrame(BUS1, &frame);


        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(DASHBOARD_BROADCAST_TASK_INTERVAL)
        );
    }
}