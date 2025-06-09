/*
 * wheelspeeds.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <main.h>
#include <semphr.h>
#include <tasks.h>
#include <wheelspeeds.h>

extern TIM_HandleTypeDef htim1;  // Front Left
extern TIM_HandleTypeDef htim3;  // Front Right
extern TIM_HandleTypeDef htim4;  // Rear Left
extern TIM_HandleTypeDef htim8;  // Rear Right

static float speedFrontLeft = 0.0f;
static float speedFrontRight = 0.0f;
static float speedRearLeft = 0.0f;
static float speedRearRight = 0.0f;
static uint16_t lastFL = 0;
static uint16_t lastFR = 0;
static uint16_t lastRL = 0;
static uint16_t lastRR = 0;

SemaphoreHandle_t wheelSpeedsMutex = NULL;
static StaticSemaphore_t wheelSpeedsMutexBuffer;

static StackType_t wheelSpeedsTaskStack[WHEEL_SPEEDS_TASK_STACK_SIZE];
static StaticTask_t wheelSpeedsTaskTCB;

void WheelSpeeds_Task(void* arguments);

void WheelSpeeds_Init(void) {
    wheelSpeedsMutex = xSemaphoreCreateBinary();
    if (wheelSpeedsMutex == NULL) {
        Error_Handler();
    }

    wheelSpeedsMutex = xSemaphoreCreateBinaryStatic(&wheelSpeedsMutexBuffer);
    if (wheelSpeedsMutex == NULL) {
        Error_Handler();
    }
    xSemaphoreGive(wheelSpeedsMutex);

    TaskHandle_t handle = xTaskCreateStatic(WheelSpeeds_Task, WHEEL_SPEEDS_TASK_NAME, WHEEL_SPEEDS_TASK_STACK_SIZE, NULL, WHEEL_SPEEDS_TASK_PRIORITY,
                                            wheelSpeedsTaskStack, &wheelSpeedsTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void WheelSpeeds_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t lastCalculateTime = xTaskGetTickCount();
    while (1) {
        if (xSemaphoreTake(wheelSpeedsMutex, portMAX_DELAY) == pdTRUE) {
            TickType_t now = xTaskGetTickCount();

            // Naive polling method, could be optimized
            uint16_t countFL = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
            uint16_t countFR = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
            uint16_t countRL = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
            uint16_t countRR = (uint16_t)__HAL_TIM_GET_COUNTER(&htim8);
            int16_t diffFL = (int16_t)((uint16_t)(countFL - lastFL));
            int16_t diffFR = (int16_t)((uint16_t)(countFR - lastFR));
            int16_t diffRL = (int16_t)((uint16_t)(countRL - lastRL));
            int16_t diffRR = (int16_t)((uint16_t)(countRR - lastRR));

            float dt = (now - lastCalculateTime) / (float)configTICK_RATE_HZ;

            speedFrontLeft = (diffFL / (float)COUNT_PER_REVOLUTION) / dt * 60.0;
            speedFrontRight = (diffFR / (float)COUNT_PER_REVOLUTION) / dt * 60.0;
            speedRearLeft = (diffRL / (float)COUNT_PER_REVOLUTION) / dt * 60.0;
            speedRearRight = (diffRR / (float)COUNT_PER_REVOLUTION) / dt * 60.0;

            lastCalculateTime = now;

            xSemaphoreGive(wheelSpeedsMutex);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(WHEEL_SPEEDS_TASK_INTERVAL));
    }
}

float wheelSpeed_FL() {
    float speed = 0.0f;
    if (xSemaphoreTake(wheelSpeedsMutex, portMAX_DELAY) == pdTRUE) {
        speed = speedFrontLeft;
        xSemaphoreGive(wheelSpeedsMutex);
    }
    return speed;
}


float wheelSpeed_FR() {
    float speed = 0.0f;
    if (xSemaphoreTake(wheelSpeedsMutex, portMAX_DELAY) == pdTRUE) {
        speed = speedFrontRight;
        xSemaphoreGive(wheelSpeedsMutex);
    }
    return speed;
}


float wheelSpeed_RL() {
    float speed = 0.0f;
    if (xSemaphoreTake(wheelSpeedsMutex, portMAX_DELAY) == pdTRUE) {
        speed = speedRearLeft;
        xSemaphoreGive(wheelSpeedsMutex);
    }
    return speed;
}


float wheelSpeed_RR() {
    float speed = 0.0f;
    if (xSemaphoreTake(wheelSpeedsMutex, portMAX_DELAY) == pdTRUE) {
        speed = speedRearRight;
        xSemaphoreGive(wheelSpeedsMutex);
    }
    return speed;
}