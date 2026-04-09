/*
 * data.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <data.h>
#include <main.h>

DataEntry CVCData[CVC_DATA_LENGTH] = {0};
DataEntry BMSData[BMS_DATA_LENGTH] = {0};
DataEntry VDMData[VDM_DATA_LENGTH] = {0};
DataEntry InverterRLData[INVERTER_DATA_LENGTH] = {0};
DataEntry InverterRRData[INVERTER_DATA_LENGTH] = {0};

static StaticSemaphore_t CVCDataMutexBuffer;
static StaticSemaphore_t BMSDataMutexBuffer;
static StaticSemaphore_t VDMDataMutexBuffer;
static StaticSemaphore_t InverterRLDataMutexBuffer;
static StaticSemaphore_t InverterRRDataMutexBuffer;
SemaphoreHandle_t CVCDataMutex = NULL;
SemaphoreHandle_t BMSDataMutex = NULL;
SemaphoreHandle_t VDMDataMutex = NULL;
SemaphoreHandle_t InverterRLDataMutex = NULL;
SemaphoreHandle_t InverterRRDataMutex = NULL;

void Data_Init(void) {
    CVCDataMutex = xSemaphoreCreateMutexStatic(&CVCDataMutexBuffer);
    BMSDataMutex = xSemaphoreCreateMutexStatic(&BMSDataMutexBuffer);
    VDMDataMutex = xSemaphoreCreateMutexStatic(&VDMDataMutexBuffer);
    InverterRLDataMutex = xSemaphoreCreateMutexStatic(&InverterRLDataMutexBuffer);
    InverterRRDataMutex = xSemaphoreCreateMutexStatic(&InverterRRDataMutexBuffer);

    if (CVCDataMutex == NULL || BMSDataMutex == NULL || VDMDataMutex == NULL || InverterRLDataMutex == NULL || InverterRRDataMutex == NULL) {
        Error_Handler();  // Handle mutex creation failure
    }

    // Initialize data entries
    for (int i = 0; i < CVC_DATA_LENGTH; i++) {
        CVCData[i].data = 0;
        CVCData[i].time = 0;
    }
    for (int i = 0; i < BMS_DATA_LENGTH; i++) {
        BMSData[i].data = 0;
        BMSData[i].time = 0;
    }
    for (int i = 0; i < VDM_DATA_LENGTH; i++) {
        VDMData[i].data = 0;
        VDMData[i].time = 0;
    }
    for (int i = 0; i < INVERTER_DATA_LENGTH; i++) {
        InverterRLData[i].data = 0;
        InverterRLData[i].time = 0;
    }
    for (int i = 0; i < INVERTER_DATA_LENGTH; i++) {
        InverterRRData[i].data = 0;
        InverterRRData[i].time = 0;
    }
}

void CVCDataSet(CVC_Data_Index index, uint32_t value) {
    if (xSemaphoreTake(CVCDataMutex, portMAX_DELAY) == pdTRUE) {
        CVCData[index].data = value;
        CVCData[index].time = xTaskGetTickCount();
        xSemaphoreGive(CVCDataMutex);
    }
}

DataEntry CVCDataGet(CVC_Data_Index index) {
    DataEntry entry = {.data = 0, .time = 0};
    if (xSemaphoreTake(CVCDataMutex, portMAX_DELAY) == pdTRUE) {
        entry = CVCData[index];
        xSemaphoreGive(CVCDataMutex);
    }
    return entry;
}

void BMSDataSet(BMS_Data_Index index, uint32_t value) {
    if (xSemaphoreTake(BMSDataMutex, portMAX_DELAY) == pdTRUE) {
        BMSData[index].data = value;
        BMSData[index].time = xTaskGetTickCount();
        xSemaphoreGive(BMSDataMutex);
    }
}

DataEntry BMSDataGet(BMS_Data_Index index) {
    DataEntry entry = {.data = 0, .time = 0};
    if (xSemaphoreTake(BMSDataMutex, portMAX_DELAY) == pdTRUE) {
        entry = BMSData[index];
        xSemaphoreGive(BMSDataMutex);
    }
    return entry;
}

void VDMDataSet(VDM_Data_Index index, uint32_t value) {
    if (xSemaphoreTake(VDMDataMutex, portMAX_DELAY) == pdTRUE) {
        VDMData[index].data = value;
        VDMData[index].time = xTaskGetTickCount();
        xSemaphoreGive(VDMDataMutex);
    }
}

DataEntry VDMDataGet(VDM_Data_Index index) {
    DataEntry entry = {.data = 0, .time = 0};
    if (xSemaphoreTake(VDMDataMutex, portMAX_DELAY) == pdTRUE) {
        entry = VDMData[index];
        xSemaphoreGive(VDMDataMutex);
    }
    return entry;
}

void InverterRLDataSet(Inverter_Data_Index index, uint32_t value) {
    if (xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY) == pdTRUE) {
        InverterRLData[index].data = value;
        InverterRLData[index].time = xTaskGetTickCount();
        xSemaphoreGive(InverterRLDataMutex);
    }
}

DataEntry InverterRLDataGet(Inverter_Data_Index index) {
    DataEntry entry = {.data = 0, .time = 0};
    if (xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY) == pdTRUE) {
        entry = InverterRLData[index];
        xSemaphoreGive(InverterRLDataMutex);
    }
    return entry;
}

void InverterRRDataSet(Inverter_Data_Index index, uint32_t value) {
    if (xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY) == pdTRUE) {
        InverterRRData[index].data = value;
        InverterRRData[index].time = xTaskGetTickCount();
        xSemaphoreGive(InverterRRDataMutex);
    }
}

DataEntry InverterRRDataGet(Inverter_Data_Index index) {
    DataEntry entry = {.data = 0, .time = 0};
    if (xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY) == pdTRUE) {
        entry = InverterRRData[index];
        xSemaphoreGive(InverterRRDataMutex);
    }
    return entry;
}
