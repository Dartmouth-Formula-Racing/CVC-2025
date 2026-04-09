/*
 * analogs.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <analogs.h>
#include <main.h>
#include <string.h>
#include <task.h>

static uint16_t buffer[ADC_CHANNEL_COUNT] = {0};
static uint16_t adcDMABuffer[ADC_CHANNEL_COUNT] = {0};
extern ADC_HandleTypeDef hadc1;

static volatile uint32_t analogTimeoutCount = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance != ADC1) {
        return;
    }

    UBaseType_t criticalState = taskENTER_CRITICAL_FROM_ISR();
    memcpy(buffer, adcDMABuffer, sizeof(buffer));
    taskEXIT_CRITICAL_FROM_ISR(criticalState);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance != ADC1) {
        return;
    }

    analogTimeoutCount++;
    (void)HAL_ADC_Stop_DMA(&hadc1);
    (void)HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcDMABuffer, ADC_CHANNEL_COUNT);
}

void Analogs_Init(void) {
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcDMABuffer, ADC_CHANNEL_COUNT) != HAL_OK) {
        analogTimeoutCount++;
        Error_Handler();
    }
}

// Returns the 12-bit ADC reading for the given AnalogChannel
uint16_t Analogs_ReadChannel(AnalogChannel ch) {
    uint16_t value = 0xFFFF;  // Default to 0xFFFF (invalid reading)
    if (ch >= 0 && ch < ADC_CHANNEL_COUNT) {
        taskENTER_CRITICAL();
        {
            value = buffer[(int)ch];
        }
        taskEXIT_CRITICAL();
    }
    return value;
}

uint32_t Analogs_GetTimeoutCount(void) { return analogTimeoutCount; }