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
#include <tasks.h>
#include <throttle.h>

static bool throttleValid = false;
static float throttleValue = 0.0f;

static StackType_t throttleTaskStack[THROTTLE_TASK_STACK_SIZE];
static StaticTask_t throttleTaskTCB;

void Throttle_Task(void* arguments);

void Throttle_Init(void) {
    // Initialize throttle task using static allocation
    TaskHandle_t handle =
        xTaskCreateStatic(Throttle_Task, THROTTLE_TASK_NAME, THROTTLE_TASK_STACK_SIZE, NULL, THROTTLE_TASK_PRIORITY, throttleTaskStack, &throttleTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void Throttle_Task(void* arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t throttleValidStart = lastWakeTime;
    bool plausibilityCheck = false;
    bool instantValid = false;
    while (1) {
        uint16_t apps1ADC = Analogs_ReadChannel(APPS_1);
        uint16_t apps2ADC = Analogs_ReadChannel(APPS_2);
        apps1ADC = apps1ADC < APPS1_MIN ? APPS1_MIN : apps1ADC;
        apps1ADC = apps1ADC > APPS1_MAX ? APPS1_MAX : apps1ADC;
        apps2ADC = apps2ADC < APPS2_MIN ? APPS2_MIN : apps2ADC;
        apps2ADC = apps2ADC > APPS2_MAX ? APPS2_MAX : apps2ADC;

        float apps1 = (float)(apps1ADC - APPS1_MIN) / (APPS1_MAX - APPS1_MIN);
        float apps2 = (float)(apps2ADC - APPS2_MIN) / (APPS2_MAX - APPS2_MIN);

        // Check if readings match within tolerance
        if (fabs(apps1 - apps2) < THROTTLE_TOLERANCE) {
            if (!instantValid) {
                instantValid = true;
                throttleValidStart = xTaskGetTickCount();
            }
        } else {
            instantValid = false;
        }

        // Check if throttle has been valid for the minimum time
        if (instantValid && (xTaskGetTickCount() - throttleValidStart) >= pdMS_TO_TICKS(THROTTLE_MIN_VALID_TIME)) {
            throttleValid = true;
        } else {
            throttleValid = false;
        }

        if (throttleValid) {
            if (!plausibilityCheck) {  // EV.4.7.1
                if (apps1 < 0.05) {
                    plausibilityCheck = true;
                }
            } else if (apps1 > 0.25 && Brake_GetState() == HARD_BRAKE) {
                plausibilityCheck = false;
            }
        } else {
            throttleValue = 0.0f;
        }

        if (throttleValid && plausibilityCheck) {
            throttleValue = apps1;
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(THROTTLE_TASK_INTERVAL));
    }
}

bool Throttle_Valid(void) { return throttleValid; }

float Throttle_GetValue(void) { return throttleValue; }