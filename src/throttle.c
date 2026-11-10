/*
 * throttle.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <analogs.h>
#include <main.h>
#include <math.h>
#include <misc.h>
#include <stdbool.h>
#include <semphr.h>
#include <task.h>
#include <tasks.h>
#include <throttle.h>

static bool throttleValid = false;
static float throttleValue = 0.0f;

static uint16_t apps1MedianHistory[3] = {0U, 0U, 0U};
static uint8_t apps1MedianIndex = 0U;

static uint16_t MedianOfThree(uint16_t a, uint16_t b, uint16_t c) {
    uint16_t sorted[3] = {a, b, c};
    uint16_t temp = 0U;

    if (sorted[0] > sorted[1]) {
        temp = sorted[0];
        sorted[0] = sorted[1];
        sorted[1] = temp;
    }
    if (sorted[1] > sorted[2]) {
        temp = sorted[1];
        sorted[1] = sorted[2];
        sorted[2] = temp;
    }
    if (sorted[0] > sorted[1]) {
        temp = sorted[0];
        sorted[0] = sorted[1];
        sorted[1] = temp;
    }

    return sorted[1];
}

static SemaphoreHandle_t throttleMutex = NULL;
static StaticSemaphore_t throttleMutexBuffer;

static StackType_t throttleTaskStack[THROTTLE_TASK_STACK_SIZE];
static StaticTask_t throttleTaskTCB;

void Throttle_Task(void* arguments);

void Throttle_Init(void) {
    throttleMutex = xSemaphoreCreateMutexStatic(&throttleMutexBuffer);
    if (throttleMutex == NULL) {
        Error_Handler();
    }
    TaskHandle_t handle =
        xTaskCreateStatic(Throttle_Task, THROTTLE_TASK_NAME, THROTTLE_TASK_STACK_SIZE, NULL, THROTTLE_TASK_PRIORITY, throttleTaskStack, &throttleTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void Throttle_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    bool plausibilityCheck = false;
    bool instantValid = false;
    while (1) {
        uint16_t apps1ADC = Analogs_ReadChannel(APPS_1);
        uint16_t apps2ADC = Analogs_ReadChannel(APPS_2);
        apps1ADC = apps1ADC < APPS1_MIN ? APPS1_MIN : apps1ADC;
        apps1ADC = apps1ADC > APPS1_MAX ? APPS1_MAX : apps1ADC;
        apps2ADC = apps2ADC < APPS2_MIN ? APPS2_MIN : apps2ADC;
        apps2ADC = apps2ADC > APPS2_MAX ? APPS2_MAX : apps2ADC;

        apps1MedianHistory[apps1MedianIndex] = apps1ADC;
        apps1MedianIndex = (apps1MedianIndex + 1U) % 3U;

        uint16_t apps1FilteredADC = MedianOfThree(apps1MedianHistory[0], apps1MedianHistory[1], apps1MedianHistory[2]);

        float apps1 = (float)(apps1FilteredADC - APPS1_MIN) / (APPS1_MAX - APPS1_MIN);
        float apps2 = (float)(apps2ADC - APPS2_MIN) / (APPS2_MAX - APPS2_MIN);

        // Check if readings match within tolerance
        if (fabs(apps1 - apps2) < THROTTLE_TOLERANCE) {
            if (!instantValid) {
                instantValid = true;
            }
        } else {
            instantValid = false;
        }

        // Check if throttle has been valid for the minimum time
        // if (instantValid && (xTaskGetTickCount() - throttleValidStart) >= pdMS_TO_TICKS(THROTTLE_MIN_VALID_TIME)) {
        //     throttleValid = true;
        // } else {
        //     throttleValid = false;
        // }

        if (instantValid) {
            throttleValid = true;
        }
        if (!plausibilityCheck) {  // EV.4.7.1
            if (apps1 < 0.05) {
                plausibilityCheck = true;
            }
        } else if (apps1 > 0.25 && Brake_GetState() == HARD_BRAKE) {
            plausibilityCheck = false;
        }

        if (!plausibilityCheck) {
            plausibilityCheck = true;  // Only applies for FSAE, skip for FH&E
        }

        if (xSemaphoreTake(throttleMutex, portMAX_DELAY) == pdTRUE) {
            if (throttleValid && plausibilityCheck) {
                throttleValue = apps1;
            } else {
                throttleValue = 0.0f;
            }
            xSemaphoreGive(throttleMutex);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(THROTTLE_TASK_INTERVAL));
    }
}

bool Throttle_Valid(void) {
    bool valid = false;
    if (xSemaphoreTake(throttleMutex, portMAX_DELAY) == pdTRUE) {
        valid = throttleValid;
        xSemaphoreGive(throttleMutex);
    }
    return valid;
}

float Throttle_GetValue(void) {
    float value = 0.0f;
    if (xSemaphoreTake(throttleMutex, portMAX_DELAY) == pdTRUE) {
        value = throttleValue;
        xSemaphoreGive(throttleMutex);
    }
    return value;
}