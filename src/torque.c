/*
 * torque.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <analogs.h>
#include <can.h>
#include <data.h>
#include <misc.h>
#include <statemachine.h>
#include <stdbool.h>
#include <task.h>
#include <tasks.h>
#include <throttle.h>
#include <torque.h>

static float torqueRearLeft = 0.0f;
static float torqueRearRight = 0.0f;
static float torqueFrontLeft = 0.0f;
static float torqueFrontRight = 0.0f;

static StackType_t torqueCalculateTaskStack[TORQUE_CALCULATE_TASK_STACK_SIZE];
static StaticTask_t torqueCalculateTaskTCB;
static StackType_t torqueCommandTaskStack[TORQUE_COMMAND_TASK_STACK_SIZE];
static StaticTask_t torqueCommandTaskTCB;
static StackType_t torqueLimitTaskStack[TORQUE_LIMIT_TASK_STACK_SIZE];
static StaticTask_t torqueLimitTaskTCB;

void Torque_LimitInverters(void);
void Torque_CalculateTask(void* arguments);
void Torque_CommandTask(void* arguments);
void Torque_LimitTask(void* arguments);
int Error_Handler(void);

void Torque_Init(void) {
    TaskHandle_t handle = xTaskCreateStatic(Torque_CalculateTask, TORQUE_CALCULATE_TASK_NAME, TORQUE_CALCULATE_TASK_STACK_SIZE, NULL,
                                            TORQUE_CALCULATE_TASK_PRIORITY, torqueCalculateTaskStack, &torqueCalculateTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
    handle = xTaskCreateStatic(Torque_CommandTask, TORQUE_COMMAND_TASK_NAME, TORQUE_COMMAND_TASK_STACK_SIZE, NULL, TORQUE_COMMAND_TASK_PRIORITY,
                               torqueCommandTaskStack, &torqueCommandTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }

    handle = xTaskCreateStatic(Torque_LimitTask, TORQUE_LIMIT_TASK_NAME, TORQUE_LIMIT_TASK_STACK_SIZE, NULL, TORQUE_LIMIT_TASK_PRIORITY,
                               torqueLimitTaskStack, &torqueLimitTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void Torque_CalculateTask(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1) {
        float throttleValue = Throttle_GetValue();
        volatile uint16_t steeringAngleADC = Analogs_ReadChannel(Steering_Angle);
        steeringAngleADC = steeringAngleADC < STEERING_LEFT_LIMIT ? STEERING_LEFT_LIMIT : steeringAngleADC;
        steeringAngleADC = steeringAngleADC > STEERING_RIGHT_LIMIT ? STEERING_RIGHT_LIMIT : steeringAngleADC;
        float steeringAngle = 2.0f * (float)(steeringAngleADC - STEERING_LEFT_LIMIT) / (STEERING_RIGHT_LIMIT - STEERING_LEFT_LIMIT) - 1.0f;

        torqueRearLeft = throttleValue * TORQUE_MAX;
        torqueRearRight = throttleValue * TORQUE_MAX;
        torqueFrontLeft = throttleValue * TORQUE_MAX;
        torqueFrontRight = throttleValue * TORQUE_MAX;

        if (StateMachine_GetDriveState() == REVERSE) {
            torqueRearLeft *= REVERSE_TORQUE_LIMIT;
            torqueRearRight *= REVERSE_TORQUE_LIMIT;
            torqueFrontLeft *= REVERSE_TORQUE_LIMIT;
            torqueFrontRight *= REVERSE_TORQUE_LIMIT;
        }

        if (steeringAngle > 0.0f) {
            torqueRearLeft -= TORQUE_VECTORING_GAIN * steeringAngle * torqueRearLeft;
            torqueFrontLeft -= TORQUE_VECTORING_GAIN * steeringAngle * torqueFrontLeft;
        } else {
            torqueRearRight -= TORQUE_VECTORING_GAIN * steeringAngle * torqueRearRight;
            torqueFrontRight -= TORQUE_VECTORING_GAIN * steeringAngle * torqueFrontRight;
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TORQUE_CALCULATE_TASK_INTERVAL));
    }
}

void Torque_CommandTask(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1) {
        CAN_Frame RLFrame = {0};
        CAN_Frame RRFrame = {0};
        // CAN_Frame FLFrame = {0};
        // CAN_Frame FRFrame = {0};

        RLFrame.header.tx.StdId = InverterRL_STD(0x10);
        RLFrame.header.tx.ExtId = InverterRL_EXT(0x10);
        RLFrame.header.tx.IDE = CAN_ID_STD;
        RLFrame.header.tx.RTR = CAN_RTR_DATA;
        RLFrame.header.tx.DLC = 8;

        RRFrame.header.tx.StdId = InverterRR_STD(0x10);
        RRFrame.header.tx.ExtId = InverterRR_EXT(0x10);
        RRFrame.header.tx.IDE = CAN_ID_STD;
        RRFrame.header.tx.RTR = CAN_RTR_DATA;
        RRFrame.header.tx.DLC = 8;

        // Torque command
        RLFrame.data[0] = (uint8_t)((int16_t)torqueRearLeft & 0xFF);
        RLFrame.data[1] = (uint8_t)(((int16_t)torqueRearLeft >> 8) & 0xFF);
        RRFrame.data[0] = (uint8_t)((int16_t)torqueRearRight & 0xFF);
        RRFrame.data[1] = (uint8_t)(((int16_t)torqueRearRight >> 8) & 0xFF);

        // Speed command
        RLFrame.data[2] = 0;
        RLFrame.data[3] = 0;
        RRFrame.data[2] = 0;
        RRFrame.data[3] = 0;

        // Direction command
        if (StateMachine_GetDriveState() == REVERSE) {
            RLFrame.data[4] = 0;  // Reverse left motor
            RRFrame.data[4] = 1;
        } else {
            RLFrame.data[4] = 1;
            RRFrame.data[4] = 0;  // Reverse right motor
        }

        // Inverter enable, discharge, & speed mode bits
        RLFrame.data[5] = 0;
        RRFrame.data[5] = 0;

        if (StateMachine_GetDriveState() != NEUTRAL) {
            // Enable inverters
            RLFrame.data[5] |= 0x01;  // Enable left inverter
            RRFrame.data[5] |= 0x01;  // Enable right inverter
        }
        // Other bits are 0, we aren't using active discharge or speed mode

        // Commanded torque limit, set to 0 to use EEPROM values
        RLFrame.data[6] = 0;
        RLFrame.data[7] = 0;
        RRFrame.data[6] = 0;
        RRFrame.data[7] = 0;

        CAN_SendFrame(BUS2, &RLFrame);
        CAN_SendFrame(BUS2, &RRFrame);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TORQUE_COMMAND_TASK_INTERVAL));
    }
}

void Torque_LimitTask(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1) {
        Torque_LimitInverters();
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TORQUE_LIMIT_TASK_INTERVAL));
    }
}

void Torque_LimitInverters(void) {
    // Cascadia CAN protocol 6.3 section 2.5
    // Limit discharge and charge current using message 0x202
    // Bytes [0, 1] - Discharge current limit (A)
    // Bytes [2, 3] - Charge current limit (A)
    // Does not include traction control or torque vectoring yet, assumes even power split
    float busVoltage = (float)BMSDataGet(BMS_TOTAL_VOLTAGE).data / 100.0f;

    float maxDischarge = (MAX_POWER * 1000.0f / busVoltage) / 2;
    if (maxDischarge > INVERTER_CURRENT_LIMIT) {
        maxDischarge = INVERTER_CURRENT_LIMIT;
    }

    float maxCharge = MAX_REGEN_CURRENT;
    if (maxCharge > INVERTER_CURRENT_LIMIT) {
        maxCharge = INVERTER_CURRENT_LIMIT;
    }

    float maxBSPD = (MAX_BSPD_POWER * 1000.0f / busVoltage) / 2;
    if (Brake_GetState() == HARD_BRAKE) {
        maxDischarge = maxBSPD;
    }

    CAN_Frame RLFrame = {0};
    CAN_Frame RRFrame = {0};

    RLFrame.header.tx.StdId = InverterRL_STD(0x02);
    RLFrame.header.tx.ExtId = InverterRL_EXT(0x02);
    RLFrame.header.tx.IDE = CAN_ID_STD;
    RLFrame.header.tx.RTR = CAN_RTR_DATA;
    RLFrame.header.tx.DLC = 8;

    RRFrame.header.tx.StdId = InverterRR_STD(0x02);
    RRFrame.header.tx.ExtId = InverterRR_EXT(0x02);
    RRFrame.header.tx.IDE = CAN_ID_STD;
    RRFrame.header.tx.RTR = CAN_RTR_DATA;
    RRFrame.header.tx.DLC = 8;

    RLFrame.data[0] = (uint8_t)((uint16_t)maxDischarge & 0xFF);
    RLFrame.data[1] = (uint8_t)(((uint16_t)maxDischarge >> 8) & 0xFF);
    RLFrame.data[2] = (uint8_t)((uint16_t)maxCharge & 0xFF);
    RLFrame.data[3] = (uint8_t)(((uint16_t)maxCharge >> 8) & 0xFF);
    RLFrame.data[4] = 0;
    RLFrame.data[5] = 0;
    RLFrame.data[6] = 0;
    RLFrame.data[7] = 0;

    RRFrame.data[0] = (uint8_t)((uint16_t)maxDischarge & 0xFF);
    RRFrame.data[1] = (uint8_t)(((uint16_t)maxDischarge >> 8) & 0xFF);
    RRFrame.data[2] = (uint8_t)((uint16_t)maxCharge & 0xFF);
    RRFrame.data[3] = (uint8_t)(((uint16_t)maxCharge >> 8) & 0xFF);
    RRFrame.data[4] = 0;
    RRFrame.data[5] = 0;
    RRFrame.data[6] = 0;
    RRFrame.data[7] = 0;

    CAN_SendFrame(BUS2, &RLFrame);
    CAN_SendFrame(BUS2, &RRFrame);
}

float Torque_RearLeft(void) { return torqueRearLeft; }
float Torque_RearRight(void) { return torqueRearRight; }
float Torque_FrontLeft(void) { return torqueFrontLeft; }
float Torque_FrontRight(void) { return torqueFrontRight; }