/*
 * analogs.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <analogs.h>
#include <main.h>
#include <semphr.h>
#include <tasks.h>

static uint16_t buffer[ADC_CHANNEL_COUNT] = {0};
// static uint16_t adcDMABuffer[ADC_CHANNEL_COUNT] = {0};
SemaphoreHandle_t analogMutex = NULL;
static StaticSemaphore_t analogMutexBuffer;
extern ADC_HandleTypeDef hadc1;

static StackType_t analogsReadTaskStack[ANALOG_READ_TASK_STACK_SIZE];
static StaticTask_t analogsReadTaskTCB;

void Analogs_Read_Task(void *arguments);

// TODO: Put new ADC readings in a queue
// Queue is mutexed and values popped out onto buffer
// Allows full speed ADC readings without deadlock

// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
//     // Protect buffer with mutex
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     if (analogMutex != NULL) {
//         if (xPortIsInsideInterrupt()) {
//             if (xSemaphoreTakeFromISR(analogMutex, &xHigherPriorityTaskWoken) == pdTRUE) {
//                 for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
//                     buffer[i] = adcDMABuffer[i];
//                 }
//                 xSemaphoreGiveFromISR(analogMutex, &xHigherPriorityTaskWoken);
//                 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//             }
//         } else {
//             if (xSemaphoreTake(analogMutex, portMAX_DELAY) == pdTRUE) {
//                 for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
//                     buffer[i] = adcDMABuffer[i];
//                 }
//                 xSemaphoreGive(analogMutex);
//             }
//         }
//     }
// }

void Analogs_Init(void) {
    analogMutex = xSemaphoreCreateBinaryStatic(&analogMutexBuffer);
    if (analogMutex == NULL) {
        Error_Handler();
    }
    xSemaphoreGive(analogMutex);  // Initialize semaphore as available

    TaskHandle_t handle = xTaskCreateStatic(Analogs_Read_Task, ANALOG_READ_TASK_NAME, ANALOG_READ_TASK_STACK_SIZE, NULL, ANALOG_READ_TASK_PRIORITY,
                                            analogsReadTaskStack, &analogsReadTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }
}

void Analogs_Read_Task(void *arguments) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (xSemaphoreTake(analogMutex, portMAX_DELAY) == pdTRUE) {
            // Start one full ADC sequence, then read each rank in order.
            HAL_ADC_Start(&hadc1);
            for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
                // Make sure the total polling time does not exceed ANALOG_READ_TASK_INTERVAL
                // 1ms timeout instead of HAL_MAX DELAY blocking forever
                // buffer[i] = 0;
                if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) {
                    buffer[i] = HAL_ADC_GetValue(&hadc1) & 0x0FFF;}
                }
            HAL_ADC_Stop(&hadc1);
            xSemaphoreGive(analogMutex);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(ANALOG_READ_TASK_INTERVAL));  // Adjust delay as needed
    }
}

// Returns the 12-bit ADC reading for the given AnalogChannel
uint16_t Analogs_ReadChannel(AnalogChannel ch) {
    uint16_t value = 0xFFFF;  // Default to 0xFFFF (invalid reading)
    if (ch >= 0 && ch < ADC_CHANNEL_COUNT) {
        if (xSemaphoreTake(analogMutex, portMAX_DELAY) == pdTRUE) {
            value = buffer[(int)ch];
            xSemaphoreGive(analogMutex);
        }
    }
    return value;
}