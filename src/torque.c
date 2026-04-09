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
#include <semphr.h>
#include <statemachine.h>
#include <stdbool.h>
#include <task.h>
#include <tasks.h>
#include <throttle.h>
#include <torque.h>

#if CAN_INVERTER_USE_EXT
#define INVERTER_CAN_IDE CAN_ID_EXT
#else
#define INVERTER_CAN_IDE CAN_ID_STD
#endif

static float torqueRearLeft = 0.0f;
static float torqueRearRight = 0.0f;
static float torqueFrontLeft = 0.0f;
static float torqueFrontRight = 0.0f;
static SemaphoreHandle_t torqueDataMutex = NULL;
static StaticSemaphore_t torqueDataMutexBuffer;

static StackType_t torqueCalculateTaskStack[TORQUE_CALCULATE_TASK_STACK_SIZE];
static StaticTask_t torqueCalculateTaskTCB;
static StackType_t torqueCommandTaskStack[TORQUE_COMMAND_TASK_STACK_SIZE];
static StaticTask_t torqueCommandTaskTCB;
static StackType_t torqueLimitTaskStack[TORQUE_LIMIT_TASK_STACK_SIZE];
static StaticTask_t torqueLimitTaskTCB;

void Torque_CalculateTask(void* arguments);
void Torque_CommandTask(void* arguments);
void Torque_LimitTask(void* arguments);
int Error_Handler(void);

void Torque_SendInverterFaultClear(void) {
    CAN_Frame RLResetFrame = {0};
    CAN_Frame RRResetFrame = {0};

    RLResetFrame.header.tx.StdId = InverterRL_STD(0x21);
    RLResetFrame.header.tx.ExtId = InverterRL_EXT(0x21);
    RLResetFrame.header.tx.IDE = INVERTER_CAN_IDE;
    RLResetFrame.header.tx.RTR = CAN_RTR_DATA;
    RLResetFrame.header.tx.DLC = 8;

    RRResetFrame.header.tx.StdId = InverterRR_STD(0x21);
    RRResetFrame.header.tx.ExtId = InverterRR_EXT(0x21);
    RRResetFrame.header.tx.IDE = INVERTER_CAN_IDE;
    RRResetFrame.header.tx.RTR = CAN_RTR_DATA;
    RRResetFrame.header.tx.DLC = 8;

    // Cascadia reset/fault clear: parameter 20, write=1.
    RLResetFrame.data[0] = 20;
    RLResetFrame.data[1] = 0;
    RLResetFrame.data[2] = 1;
    RRResetFrame.data[0] = 20;
    RRResetFrame.data[1] = 0;
    RRResetFrame.data[2] = 1;

    CAN_SendFrame(BUS2, &RLResetFrame);
    CAN_SendFrame(BUS2, &RRResetFrame);
}

void Torque_Init(void) {
    torqueDataMutex = xSemaphoreCreateMutexStatic(&torqueDataMutexBuffer);
    if (torqueDataMutex == NULL) {
        Error_Handler();
    }

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

    handle = xTaskCreateStatic(Torque_LimitTask, TORQUE_LIMIT_TASK_NAME, TORQUE_LIMIT_TASK_STACK_SIZE, NULL, TORQUE_LIMIT_TASK_PRIORITY, torqueLimitTaskStack,
                               &torqueLimitTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void Torque_CalculateTask(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1) {
        float rearLeft = 0.0f;
        float rearRight = 0.0f;
        float frontLeft = 0.0f;
        float frontRight = 0.0f;

        float throttle = Throttle_GetValue();

        volatile uint16_t steeringAngleADC = Analogs_ReadChannel(Steering_Angle);
        steeringAngleADC = steeringAngleADC < STEERING_LEFT_LIMIT ? STEERING_LEFT_LIMIT : steeringAngleADC;
        steeringAngleADC = steeringAngleADC > STEERING_RIGHT_LIMIT ? STEERING_RIGHT_LIMIT : steeringAngleADC;
        float steeringAngle = 2.0f * (float)(steeringAngleADC - STEERING_LEFT_LIMIT) / (STEERING_RIGHT_LIMIT - STEERING_LEFT_LIMIT) - 1.0f;

        rearLeft = throttle * TORQUE_MAX * 10.0;
        rearRight = throttle * TORQUE_MAX * 10.0;
        frontLeft = throttle * TORQUE_MAX * 10.0;
        frontRight = throttle * TORQUE_MAX * 10.0;

        if (StateMachine_GetDriveState() == REVERSE) {
            rearLeft *= REVERSE_TORQUE_LIMIT;
            rearRight *= REVERSE_TORQUE_LIMIT;
            frontLeft *= REVERSE_TORQUE_LIMIT;
            frontRight *= REVERSE_TORQUE_LIMIT;
        }

        if (steeringAngle > 0.0f) {
            rearLeft -= TORQUE_VECTORING_GAIN * steeringAngle * rearLeft;
            frontLeft -= TORQUE_VECTORING_GAIN * steeringAngle * frontLeft;
        } else {
            rearRight -= TORQUE_VECTORING_GAIN * steeringAngle * rearRight;
            frontRight -= TORQUE_VECTORING_GAIN * steeringAngle * frontRight;
        }

        if (xSemaphoreTake(torqueDataMutex, portMAX_DELAY) == pdTRUE) {
            torqueRearLeft = rearLeft;
            torqueRearRight = rearRight;
            torqueFrontLeft = frontLeft;
            torqueFrontRight = frontRight;
            xSemaphoreGive(torqueDataMutex);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TORQUE_CALCULATE_TASK_INTERVAL));
    }
}

void Torque_CommandTask(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1) {
        TorqueValues torqueValues = Torque_GetValues();
        DriveState driveState = StateMachine_GetDriveState();

        CAN_Frame RLFrame = {0};
        CAN_Frame RRFrame = {0};
        // CAN_Frame FLFrame = {0};
        // CAN_Frame FRFrame = {0};

        RLFrame.header.tx.StdId = InverterRL_STD(0x10);
        RLFrame.header.tx.ExtId = InverterRL_EXT(0x10);
        RLFrame.header.tx.IDE = INVERTER_CAN_IDE;
        RLFrame.header.tx.RTR = CAN_RTR_DATA;
        RLFrame.header.tx.DLC = 8;

        RRFrame.header.tx.StdId = InverterRR_STD(0x10);
        RRFrame.header.tx.ExtId = InverterRR_EXT(0x10);
        RRFrame.header.tx.IDE = INVERTER_CAN_IDE;
        RRFrame.header.tx.RTR = CAN_RTR_DATA;
        RRFrame.header.tx.DLC = 8;

        // Torque command
        RLFrame.data[0] = (uint8_t)((int16_t)torqueValues.rearLeft & 0xFF);
        RLFrame.data[1] = (uint8_t)(((int16_t)torqueValues.rearLeft >> 8) & 0xFF);
        RRFrame.data[0] = (uint8_t)((int16_t)torqueValues.rearRight & 0xFF);
        RRFrame.data[1] = (uint8_t)(((int16_t)torqueValues.rearRight >> 8) & 0xFF);

        // Speed command
        RLFrame.data[2] = 0;
        RLFrame.data[3] = 0;
        RRFrame.data[2] = 0;
        RRFrame.data[3] = 0;

        // Direction command
        if (driveState == REVERSE) {
            RLFrame.data[4] = 0;  // Reverse left motor
            RRFrame.data[4] = 1;
        } else {
            RLFrame.data[4] = 1;
            RRFrame.data[4] = 0;  // Reverse right motor
        }

        // Inverter enable, discharge, & speed mode bits
        RLFrame.data[5] = 0;
        RRFrame.data[5] = 0;

        if (driveState != NEUTRAL) {
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

        // float maxCharge = MAX_REGEN_CURRENT;
        // if (maxCharge > INVERTER_CURRENT_LIMIT) {
        //     maxCharge = INVERTER_CURRENT_LIMIT;
        // }
        // disable regen for now, set charge limit to 0
        float maxCharge = 0.0f;

        float maxBSPD = (MAX_BSPD_POWER * 1000.0f / busVoltage) / 2;
        if (Brake_GetState() == HARD_BRAKE) {
            maxDischarge = maxBSPD;
        }

        CAN_Frame RLFrame = {0};
        CAN_Frame RRFrame = {0};

        RLFrame.header.tx.StdId = InverterRL_STD(0x02);
        RLFrame.header.tx.ExtId = InverterRL_EXT(0x02);
        RLFrame.header.tx.IDE = INVERTER_CAN_IDE;
        RLFrame.header.tx.RTR = CAN_RTR_DATA;
        RLFrame.header.tx.DLC = 8;

        RRFrame.header.tx.StdId = InverterRR_STD(0x02);
        RRFrame.header.tx.ExtId = InverterRR_EXT(0x02);
        RRFrame.header.tx.IDE = INVERTER_CAN_IDE;
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

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TORQUE_LIMIT_TASK_INTERVAL));
    }
}

TorqueValues Torque_GetValues(void) {
    TorqueValues values = {0};
    if (xSemaphoreTake(torqueDataMutex, portMAX_DELAY) == pdTRUE) {
        values.rearLeft = torqueRearLeft;
        values.rearRight = torqueRearRight;
        values.frontLeft = torqueFrontLeft;
        values.frontRight = torqueFrontRight;
        xSemaphoreGive(torqueDataMutex);
    }
    return values;
}