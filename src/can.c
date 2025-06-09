/*
 * can.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <FreeRTOS.h>
#include <can.h>
#include <main.h>
#include <queue.h>
#include <semphr.h>
#include <string.h>
#include <tasks.h>

SemaphoreHandle_t canMutex = NULL;
static StaticSemaphore_t canMutexBuffer;
QueueHandle_t canRxQueue = NULL;
QueueHandle_t can1TxQueue = NULL;
QueueHandle_t can2TxQueue = NULL;

static uint8_t canRxQueueStorageArea[CAN_BUFFER_LENGTH * sizeof(CAN_Frame)];
static StaticQueue_t canRxQueueControlBlock;
static uint8_t can1TxQueueStorageArea[CAN_BUFFER_LENGTH * sizeof(CAN_Frame)];
static StaticQueue_t can1TxQueueControlBlock;
static uint8_t can2TxQueueStorageArea[CAN_BUFFER_LENGTH * sizeof(CAN_Frame)];
static StaticQueue_t can2TxQueueControlBlock;

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

static StackType_t canRxTaskStack[CAN_RX_TASK_STACK_SIZE];
static StaticTask_t canRxTaskTCB;
static StackType_t can1TxTaskStack[CAN_1_TX_TASK_STACK_SIZE];
static StaticTask_t can1TxTaskTCB;
static StackType_t can2TxTaskStack[CAN_2_TX_TASK_STACK_SIZE];
static StaticTask_t can2TxTaskTCB;

void CAN_RXTask(void* arguments);
void CAN1_TXTask(void* arguments);
void CAN2_TXTask(void* arguments);

void CAN_Init(void) {
    canMutex = xSemaphoreCreateBinaryStatic(&canMutexBuffer);
    if (canMutex == NULL) {
        Error_Handler();
    }

    canRxQueue = xQueueCreateStatic(CAN_BUFFER_LENGTH, sizeof(CAN_Frame), canRxQueueStorageArea, &canRxQueueControlBlock);
    if (canRxQueue == NULL) {
        Error_Handler();
    }
    can1TxQueue = xQueueCreateStatic(CAN_BUFFER_LENGTH, sizeof(CAN_Frame), can1TxQueueStorageArea, &can1TxQueueControlBlock);
    if (can1TxQueue == NULL) {
        Error_Handler();
    }
    can2TxQueue = xQueueCreateStatic(CAN_BUFFER_LENGTH, sizeof(CAN_Frame), can2TxQueueStorageArea, &can2TxQueueControlBlock);
    if (can2TxQueue == NULL) {
        Error_Handler();
    }

    TaskHandle_t handle = xTaskCreateStatic(CAN_RXTask, CAN_RX_TASK_NAME, CAN_RX_TASK_STACK_SIZE, NULL, CAN_RX_TASK_PRIORITY, canRxTaskStack, &canRxTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }

    handle = xTaskCreateStatic(CAN1_TXTask, CAN_1_TX_TASK_NAME, CAN_1_TX_TASK_STACK_SIZE, NULL, CAN_1_TX_TASK_PRIORITY, can1TxTaskStack, &can1TxTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }

    handle = xTaskCreateStatic(CAN2_TXTask, CAN_2_TX_TASK_NAME, CAN_2_TX_TASK_STACK_SIZE, NULL, CAN_2_TX_TASK_PRIORITY, can2TxTaskStack, &can2TxTaskTCB);
    if (handle == NULL) {
        Error_Handler();
    }

    xSemaphoreGive(canMutex);
}

void CAN_RXTask(void* arguments) {
    while (1) {
        CAN_Frame rxFrame;
        if (xQueueReceive(canRxQueue, &rxFrame, portMAX_DELAY) == pdTRUE) {
            uint32_t canID = rxFrame.header.rx.IDE == CAN_ID_STD ? rxFrame.header.rx.StdId : rxFrame.header.rx.ExtId;
            CAN_Data_Entry* entryPtr = CAN_getPtrByID(canID);
            if (entryPtr != NULL) {
                xSemaphoreTake(canMutex, portMAX_DELAY);
                memcpy(entryPtr->data, rxFrame.data, sizeof(rxFrame.data));
                entryPtr->timestamp = rxFrame.header.rx.Timestamp;
                entryPtr->parsed = false;
                xSemaphoreGive(canMutex);
            }
        }
    }
}

void CAN1_TXTask(void* arguments) {
    while (1) {
        // Process messages to send on CAN1
        CAN_Frame txFrame;
        uint32_t txMailbox;
        if (xQueueReceive(can1TxQueue, &txFrame, portMAX_DELAY) == pdTRUE) {
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            HAL_CAN_AddTxMessage(&hcan1, &txFrame.header.tx, txFrame.data, &txMailbox);
        }
    }
}

void CAN2_TXTask(void* arguments) {
    while (1) {
        // Process messages to send on CAN2
        CAN_Frame txFrame;
        uint32_t txMailbox;
        if (xQueueReceive(can2TxQueue, &txFrame, portMAX_DELAY) == pdTRUE) {
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            HAL_CAN_AddTxMessage(&hcan2, &txFrame.header.tx, txFrame.data, &txMailbox);
        }
    }
}

CAN_Data_Entry CAN_getDataByIndex(CAN_Message_Index index) {
    if (index < 0 || index >= NUM_MESSAGES) {
        // Invalid index, return an empty entry
        CAN_Data_Entry emptyEntry = {0};
        return emptyEntry;
    }

    // Protect access to canData with mutex
    if (xSemaphoreTake(canMutex, pdMS_TO_TICKS(CAN_MAX_SEND_TIME)) == pdTRUE) {
        CAN_Data_Entry entry = canData[index];
        xSemaphoreGive(canMutex);
        return entry;
    } else {
        // Failed to acquire mutex, return an empty entry
        CAN_Data_Entry emptyEntry = {0};
        return emptyEntry;
    }
}

CAN_Data_Entry* CAN_getPtrByIndex(CAN_Message_Index index) {
    if (index < 0 || index >= NUM_MESSAGES) {
        // Invalid index, return NULL
        return NULL;
    }

    CAN_Data_Entry* entryPtr = &canData[index];
    return entryPtr;
}

CAN_Data_Entry CAN_getDataByID(uint32_t canID) {
    // Protect access to canData with mutex
    if (xSemaphoreTake(canMutex, pdMS_TO_TICKS(CAN_MAX_SEND_TIME)) == pdTRUE) {
        for (size_t i = 0; i < canDataLength; i++) {
            if (canData[i].canID == canID) {
                CAN_Data_Entry entry = canData[i];
                xSemaphoreGive(canMutex);
                return entry;
            }
        }
        xSemaphoreGive(canMutex);
    }

    // If not found, return an empty entry
    CAN_Data_Entry emptyEntry = {0};
    return emptyEntry;
}
CAN_Data_Entry* CAN_getPtrByID(uint32_t canID) {
    // Protect access to canData with mutex
    for (size_t i = 0; i < canDataLength; i++) {
        if (canData[i].canID == canID) {
            CAN_Data_Entry* entryPtr = &canData[i];
            return entryPtr;
        }
    }

    // If not found, return NULL
    return NULL;
}

// CAN Receive Interrupt Handler
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    CAN_Frame rxFrame;
    if (hcan->Instance == CAN1) {
        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxFrame.header.rx, rxFrame.data);
    } else if (hcan->Instance == CAN2) {
        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxFrame.header.rx, rxFrame.data);
    }
    rxFrame.header.rx.Timestamp = pdTICKS_TO_MS(xTaskGetTickCountFromISR());

    xQueueSendFromISR(canRxQueue, &rxFrame, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void CAN_SendFrame(CAN_Bus bus, CAN_Frame* frame) {
    if (bus == BUS1) {
        xQueueSend(can1TxQueue, frame, pdMS_TO_TICKS(CAN_MAX_SEND_TIME));
    } else if (bus == BUS2) {
        xQueueSend(can2TxQueue, frame, pdMS_TO_TICKS(CAN_MAX_SEND_TIME));
    }
}

CAN_Data_Entry canData[] = {
    // EMUS (Standard 11-bit)
    {EMUS_STD(0), EMUS_OverallParameters, {0}, 0, false},
    {EMUS_STD(7), EMUS_DiagnosticCodes, {0}, 0, false},
    {EMUS_STD(1), EMUS_BatteryVoltageOverallParameters, {0}, 0, false},
    {EMUS_STD(2), EMUS_CellModuleTemperatureOverallParameters, {0}, 0, false},
    {EMUS_STD(8), EMUS_CellTemperatureOverallParameters, {0}, 0, false},
    {EMUS_STD(3), EMUS_CellBalancingRateOverallParameters, {0}, 0, false},
    {EMUS_STD(5), EMUS_StateOfChargeParameters, {0}, 0, false},
    {EMUS_STD(80), EMUS_ConfigurationParameters, {0}, 0, false},
    {EMUS_STD(81), EMUS_ContactorControl, {0}, 0, false},
    {EMUS_STD(6), EMUS_EnergyParameters, {0}, 0, false},
    {EMUS_STD(85), EMUS_Events, {0}, 0, false},

    // EMUS (29-bit)
    {EMUS_EXT(0x0000), EMUS_OverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0007), EMUS_DiagnosticCodes, {0}, 0, false},
    {EMUS_EXT(0x0001), EMUS_BatteryVoltageOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0002), EMUS_CellModuleTemperatureOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0008), EMUS_CellTemperatureOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0003), EMUS_CellBalancingRateOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0500), EMUS_StateOfChargeParameters, {0}, 0, false},
    {EMUS_EXT(0x0400), EMUS_ConfigurationParameters, {0}, 0, false},
    {EMUS_EXT(0x0401), EMUS_ContactorControl, {0}, 0, false},
    {EMUS_EXT(0x0600), EMUS_EnergyParameters, {0}, 0, false},
    {EMUS_EXT(0x0405), EMUS_Events, {0}, 0, false},

    // VDM (29-bit)
    {VDM_EXT(0x0000), VDM_GPSLatitudeLongitude, {0}, 0, false},
    {VDM_EXT(0x0001), VDM_GPSData, {0}, 0, false},
    {VDM_EXT(0x0002), VDM_GPSDateTime, {0}, 0, false},
    {VDM_EXT(0x0003), VDM_AccelerationData, {0}, 0, false},
    {VDM_EXT(0x0004), VDM_YawRateData, {0}, 0, false},

    // Inverter 1 (Standard 11-bit)
    {InverterRL_STD(0), INVERTER_RL_Temp1, {0}, 0, false},
    {InverterRL_STD(1), INVERTER_RL_Temp2, {0}, 0, false},
    {InverterRL_STD(2), INVERTER_RL_Temp3TorqueShudder, {0}, 0, false},
    {InverterRL_STD(3), INVERTER_RL_AnalogInputStatus, {0}, 0, false},
    {InverterRL_STD(4), INVERTER_RL_DigitalInputStatus, {0}, 0, false},
    {InverterRL_STD(5), INVERTER_RL_MotorPositionParameters, {0}, 0, false},
    {InverterRL_STD(6), INVERTER_RL_CurrentParameters, {0}, 0, false},
    {InverterRL_STD(7), INVERTER_RL_VoltageParameters, {0}, 0, false},
    {InverterRL_STD(8), INVERTER_RL_FluxParameters, {0}, 0, false},
    {InverterRL_STD(9), INVERTER_RL_InternalVoltageParameters, {0}, 0, false},
    {InverterRL_STD(10), INVERTER_RL_InternalStateParameters, {0}, 0, false},
    {InverterRL_STD(11), INVERTER_RL_FaultCodes, {0}, 0, false},
    {InverterRL_STD(12), INVERTER_RL_TorqueTimerParameters, {0}, 0, false},
    {InverterRL_STD(13), INVERTER_RL_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRL_STD(14), INVERTER_RL_FirmwareInformation, {0}, 0, false},
    {InverterRL_STD(15), INVERTER_RL_DiagnosticData, {0}, 0, false},
    {InverterRL_STD(16), INVERTER_RL_HighSpeedParameters, {0}, 0, false},
    {InverterRL_STD(17), INVERTER_RL_TorqueCapability, {0}, 0, false},

    // Inverter 2 (Standard 11-bit)
    {InverterRR_STD(0), INVERTER_RR_Temp1, {0}, 0, false},
    {InverterRR_STD(1), INVERTER_RR_Temp2, {0}, 0, false},
    {InverterRR_STD(2), INVERTER_RR_Temp3TorqueShudder, {0}, 0, false},
    {InverterRR_STD(3), INVERTER_RR_AnalogInputStatus, {0}, 0, false},
    {InverterRR_STD(4), INVERTER_RR_DigitalInputStatus, {0}, 0, false},
    {InverterRR_STD(5), INVERTER_RR_MotorPositionParameters, {0}, 0, false},
    {InverterRR_STD(6), INVERTER_RR_CurrentParameters, {0}, 0, false},
    {InverterRR_STD(7), INVERTER_RR_VoltageParameters, {0}, 0, false},
    {InverterRR_STD(8), INVERTER_RR_FluxParameters, {0}, 0, false},
    {InverterRR_STD(9), INVERTER_RR_InternalVoltageParameters, {0}, 0, false},
    {InverterRR_STD(10), INVERTER_RR_InternalStateParameters, {0}, 0, false},
    {InverterRR_STD(11), INVERTER_RR_FaultCodes, {0}, 0, false},
    {InverterRR_STD(12), INVERTER_RR_TorqueTimerParameters, {0}, 0, false},
    {InverterRR_STD(13), INVERTER_RR_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRR_STD(14), INVERTER_RR_FirmwareInformation, {0}, 0, false},
    {InverterRR_STD(15), INVERTER_RR_DiagnosticData, {0}, 0, false},
    {InverterRR_STD(16), INVERTER_RR_HighSpeedParameters, {0}, 0, false},
    {InverterRR_STD(17), INVERTER_RR_TorqueCapability, {0}, 0, false},

    // Inverter 1 (29-bit)
    {InverterRL_EXT(0), INVERTER_RL_Temp1, {0}, 0, false},
    {InverterRL_EXT(1), INVERTER_RL_Temp2, {0}, 0, false},
    {InverterRL_EXT(2), INVERTER_RL_Temp3TorqueShudder, {0}, 0, false},
    {InverterRL_EXT(3), INVERTER_RL_AnalogInputStatus, {0}, 0, false},
    {InverterRL_EXT(4), INVERTER_RL_DigitalInputStatus, {0}, 0, false},
    {InverterRL_EXT(5), INVERTER_RL_MotorPositionParameters, {0}, 0, false},
    {InverterRL_EXT(6), INVERTER_RL_CurrentParameters, {0}, 0, false},
    {InverterRL_EXT(7), INVERTER_RL_VoltageParameters, {0}, 0, false},
    {InverterRL_EXT(8), INVERTER_RL_FluxParameters, {0}, 0, false},
    {InverterRL_EXT(9), INVERTER_RL_InternalVoltageParameters, {0}, 0, false},
    {InverterRL_EXT(10), INVERTER_RL_InternalStateParameters, {0}, 0, false},
    {InverterRL_EXT(11), INVERTER_RL_FaultCodes, {0}, 0, false},
    {InverterRL_EXT(12), INVERTER_RL_TorqueTimerParameters, {0}, 0, false},
    {InverterRL_EXT(13), INVERTER_RL_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRL_EXT(14), INVERTER_RL_FirmwareInformation, {0}, 0, false},
    {InverterRL_EXT(15), INVERTER_RL_DiagnosticData, {0}, 0, false},
    {InverterRL_EXT(16), INVERTER_RL_HighSpeedParameters, {0}, 0, false},
    {InverterRL_EXT(17), INVERTER_RL_TorqueCapability, {0}, 0, false},

    // Inverter 2 (29-bit)
    {InverterRR_EXT(0), INVERTER_RR_Temp1, {0}, 0, false},
    {InverterRR_EXT(1), INVERTER_RR_Temp2, {0}, 0, false},
    {InverterRR_EXT(2), INVERTER_RR_Temp3TorqueShudder, {0}, 0, false},
    {InverterRR_EXT(3), INVERTER_RR_AnalogInputStatus, {0}, 0, false},
    {InverterRR_EXT(4), INVERTER_RR_DigitalInputStatus, {0}, 0, false},
    {InverterRR_EXT(5), INVERTER_RR_MotorPositionParameters, {0}, 0, false},
    {InverterRR_EXT(6), INVERTER_RR_CurrentParameters, {0}, 0, false},
    {InverterRR_EXT(7), INVERTER_RR_VoltageParameters, {0}, 0, false},
    {InverterRR_EXT(8), INVERTER_RR_FluxParameters, {0}, 0, false},
    {InverterRR_EXT(9), INVERTER_RR_InternalVoltageParameters, {0}, 0, false},
    {InverterRR_EXT(10), INVERTER_RR_InternalStateParameters, {0}, 0, false},
    {InverterRR_EXT(11), INVERTER_RR_FaultCodes, {0}, 0, false},
    {InverterRR_EXT(12), INVERTER_RR_TorqueTimerParameters, {0}, 0, false},
    {InverterRR_EXT(13), INVERTER_RR_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRR_EXT(14), INVERTER_RR_FirmwareInformation, {0}, 0, false},
    {InverterRR_EXT(15), INVERTER_RR_DiagnosticData, {0}, 0, false},
    {InverterRR_EXT(16), INVERTER_RR_HighSpeedParameters, {0}, 0, false},
    {InverterRR_EXT(17), INVERTER_RR_TorqueCapability, {0}, 0, false},
};
// Must be after the canData array definition
const size_t canDataLength = sizeof(canData) / sizeof(CAN_Data_Entry);