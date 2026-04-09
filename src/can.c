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

static volatile uint32_t can1TxDropCount = 0;
static volatile uint32_t can2TxDropCount = 0;
static volatile uint32_t canRxDropCount = 0;

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

typedef struct {
    uint32_t key;
    CAN_Data_Entry* entry;
    bool used;
} CAN_ID_Hash_Entry;

#define CAN_ID_HASH_TABLE_SIZE 256U
#define CAN_ID_HASH_MASK (CAN_ID_HASH_TABLE_SIZE - 1U)

static CAN_ID_Hash_Entry canIDHashTable[CAN_ID_HASH_TABLE_SIZE];

static uint32_t CAN_MakeLookupKey(uint32_t canID, uint32_t canIDE) {
    uint32_t ideBit = (canIDE == CAN_ID_EXT) ? 1UL : 0UL;
    return (ideBit << 29U) | (canID & 0x1FFFFFFFUL);
}

static size_t CAN_HashKey(uint32_t key) {
    // Knuth multiplicative hashing; table size is power-of-two.
    return (size_t)((key * 2654435761UL) & CAN_ID_HASH_MASK);
}

static void CAN_BuildLookupTable(void) {
    memset(canIDHashTable, 0, sizeof(canIDHashTable));

    for (size_t i = 0; i < canDataLength; i++) {
        uint32_t key = CAN_MakeLookupKey(canData[i].canID, canData[i].canIDE);
        size_t slot = CAN_HashKey(key);

        for (size_t probe = 0; probe < CAN_ID_HASH_TABLE_SIZE; probe++) {
            CAN_ID_Hash_Entry* bucket = &canIDHashTable[slot];

            if (!bucket->used) {
                bucket->used = true;
                bucket->key = key;
                bucket->entry = &canData[i];
                break;
            }

            if (bucket->key == key) {
                // Keep first registration to preserve prior linear-search behavior.
                break;
            }

            slot = (slot + 1U) & CAN_ID_HASH_MASK;
            if (probe == (CAN_ID_HASH_TABLE_SIZE - 1U)) {
                Error_Handler();
            }
        }
    }
}

static CAN_Data_Entry* CAN_getPtrByIDFast(uint32_t canID, uint32_t canIDE) {
    uint32_t key = CAN_MakeLookupKey(canID, canIDE);
    size_t slot = CAN_HashKey(key);

    for (size_t probe = 0; probe < CAN_ID_HASH_TABLE_SIZE; probe++) {
        CAN_ID_Hash_Entry* bucket = &canIDHashTable[slot];

        if (!bucket->used) {
            return NULL;
        }

        if (bucket->key == key) {
            return bucket->entry;
        }

        slot = (slot + 1U) & CAN_ID_HASH_MASK;
    }

    return NULL;
}

void CAN_RXTask(void* arguments);
void CAN1_TXTask(void* arguments);
void CAN2_TXTask(void* arguments);

void CAN_Init(void) {
    canMutex = xSemaphoreCreateMutexStatic(&canMutexBuffer);
    if (canMutex == NULL) {
        Error_Handler();
    }

    CAN_BuildLookupTable();

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
}

void CAN_RXTask(void* arguments) {
    while (1) {
        CAN_Frame rxFrame;
        if (xQueueReceive(canRxQueue, &rxFrame, portMAX_DELAY) == pdTRUE) {
            uint32_t canIDE = rxFrame.header.rx.IDE;
            uint32_t canID = rxFrame.header.rx.IDE == CAN_ID_STD ? rxFrame.header.rx.StdId : rxFrame.header.rx.ExtId;
            CAN_Data_Entry* entryPtr = CAN_getPtrByID(canID, canIDE);
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
            TickType_t start = xTaskGetTickCount();
            HAL_StatusTypeDef sent = HAL_ERROR;

            do {
                if (HAL_CAN_AddTxMessage(&hcan1, &txFrame.header.tx, txFrame.data, &txMailbox) == HAL_OK) {
                    sent = HAL_OK;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(1));
            } while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(CAN_MAX_SEND_TIME));

            if (sent != HAL_OK) {
                can1TxDropCount++;
            }
        }
    }
}

void CAN2_TXTask(void* arguments) {
    while (1) {
        // Process messages to send on CAN2
        CAN_Frame txFrame;
        uint32_t txMailbox;
        if (xQueueReceive(can2TxQueue, &txFrame, portMAX_DELAY) == pdTRUE) {
            TickType_t start = xTaskGetTickCount();
            HAL_StatusTypeDef sent = HAL_ERROR;

            do {
                if (HAL_CAN_AddTxMessage(&hcan2, &txFrame.header.tx, txFrame.data, &txMailbox) == HAL_OK) {
                    sent = HAL_OK;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(1));
            } while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(CAN_MAX_SEND_TIME));

            if (sent != HAL_OK) {
                can2TxDropCount++;
            }
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

CAN_Data_Entry CAN_getDataByID(uint32_t canID, uint32_t canIDE) {
    // Protect access to canData with mutex
    if (xSemaphoreTake(canMutex, pdMS_TO_TICKS(CAN_MAX_SEND_TIME)) == pdTRUE) {
        CAN_Data_Entry* entryPtr = CAN_getPtrByIDFast(canID, canIDE);
        if (entryPtr != NULL) {
            CAN_Data_Entry entry = *entryPtr;
            xSemaphoreGive(canMutex);
            return entry;
        }
        xSemaphoreGive(canMutex);
    }

    // If not found, return an empty entry
    CAN_Data_Entry emptyEntry = {0};
    return emptyEntry;
}
CAN_Data_Entry* CAN_getPtrByID(uint32_t canID, uint32_t canIDE) { return CAN_getPtrByIDFast(canID, canIDE); }

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

    if (xQueueSendFromISR(canRxQueue, &rxFrame, &xHigherPriorityTaskWoken) != pdTRUE) {
        canRxDropCount++;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void CAN_SendFrame(CAN_Bus bus, CAN_Frame* frame) {
    if (bus == BUS1) {
        if (xQueueSend(can1TxQueue, frame, pdMS_TO_TICKS(CAN_TX_QUEUE_TIMEOUT_MS)) != pdTRUE) {
            can1TxDropCount++;
        }
    } else if (bus == BUS2) {
        if (xQueueSend(can2TxQueue, frame, pdMS_TO_TICKS(CAN_TX_QUEUE_TIMEOUT_MS)) != pdTRUE) {
            can2TxDropCount++;
        }
    }
}

uint32_t CAN_GetBus1TxDropCount(void) { return can1TxDropCount; }
uint32_t CAN_GetBus2TxDropCount(void) { return can2TxDropCount; }
uint32_t CAN_GetRxDropCount(void) { return canRxDropCount; }

CAN_Data_Entry canData[] = {
    // EMUS (Standard 11-bit)
    {EMUS_STD(0), CAN_ID_STD, EMUS_OverallParameters, {0}, 0, false},
    {EMUS_STD(7), CAN_ID_STD, EMUS_DiagnosticCodes, {0}, 0, false},
    {EMUS_STD(1), CAN_ID_STD, EMUS_BatteryVoltageOverallParameters, {0}, 0, false},
    {EMUS_STD(2), CAN_ID_STD, EMUS_CellModuleTemperatureOverallParameters, {0}, 0, false},
    {EMUS_STD(8), CAN_ID_STD, EMUS_CellTemperatureOverallParameters, {0}, 0, false},
    {EMUS_STD(3), CAN_ID_STD, EMUS_CellBalancingRateOverallParameters, {0}, 0, false},
    {EMUS_STD(5), CAN_ID_STD, EMUS_StateOfChargeParameters, {0}, 0, false},
    {EMUS_STD(80), CAN_ID_STD, EMUS_ConfigurationParameters, {0}, 0, false},
    {EMUS_STD(81), CAN_ID_STD, EMUS_ContactorControl, {0}, 0, false},
    {EMUS_STD(6), CAN_ID_STD, EMUS_EnergyParameters, {0}, 0, false},
    {EMUS_STD(85), CAN_ID_STD, EMUS_Events, {0}, 0, false},

    // EMUS (29-bit)
    {EMUS_EXT(0x0000), CAN_ID_EXT, EMUS_OverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0007), CAN_ID_EXT, EMUS_DiagnosticCodes, {0}, 0, false},
    {EMUS_EXT(0x0001), CAN_ID_EXT, EMUS_BatteryVoltageOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0002), CAN_ID_EXT, EMUS_CellModuleTemperatureOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0008), CAN_ID_EXT, EMUS_CellTemperatureOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0003), CAN_ID_EXT, EMUS_CellBalancingRateOverallParameters, {0}, 0, false},
    {EMUS_EXT(0x0500), CAN_ID_EXT, EMUS_StateOfChargeParameters, {0}, 0, false},
    {EMUS_EXT(0x0400), CAN_ID_EXT, EMUS_ConfigurationParameters, {0}, 0, false},
    {EMUS_EXT(0x0401), CAN_ID_EXT, EMUS_ContactorControl, {0}, 0, false},
    {EMUS_EXT(0x0600), CAN_ID_EXT, EMUS_EnergyParameters, {0}, 0, false},
    {EMUS_EXT(0x0405), CAN_ID_EXT, EMUS_Events, {0}, 0, false},

    // VDM (29-bit)
    {VDM_EXT(0x0000), CAN_ID_EXT, VDM_GPSLatitudeLongitude, {0}, 0, false},
    {VDM_EXT(0x0001), CAN_ID_EXT, VDM_GPSData, {0}, 0, false},
    {VDM_EXT(0x0002), CAN_ID_EXT, VDM_GPSDateTime, {0}, 0, false},
    {VDM_EXT(0x0003), CAN_ID_EXT, VDM_AccelerationData, {0}, 0, false},
    {VDM_EXT(0x0004), CAN_ID_EXT, VDM_YawRateData, {0}, 0, false},

    // Inverter 1 (Standard 11-bit)
    {InverterRL_STD(0), CAN_ID_STD, INVERTER_RL_Temp1, {0}, 0, false},
    {InverterRL_STD(1), CAN_ID_STD, INVERTER_RL_Temp2, {0}, 0, false},
    {InverterRL_STD(2), CAN_ID_STD, INVERTER_RL_Temp3TorqueShudder, {0}, 0, false},
    {InverterRL_STD(3), CAN_ID_STD, INVERTER_RL_AnalogInputStatus, {0}, 0, false},
    {InverterRL_STD(4), CAN_ID_STD, INVERTER_RL_DigitalInputStatus, {0}, 0, false},
    {InverterRL_STD(5), CAN_ID_STD, INVERTER_RL_MotorPositionParameters, {0}, 0, false},
    {InverterRL_STD(6), CAN_ID_STD, INVERTER_RL_CurrentParameters, {0}, 0, false},
    {InverterRL_STD(7), CAN_ID_STD, INVERTER_RL_VoltageParameters, {0}, 0, false},
    {InverterRL_STD(8), CAN_ID_STD, INVERTER_RL_FluxParameters, {0}, 0, false},
    {InverterRL_STD(9), CAN_ID_STD, INVERTER_RL_InternalVoltageParameters, {0}, 0, false},
    {InverterRL_STD(10), CAN_ID_STD, INVERTER_RL_InternalStateParameters, {0}, 0, false},
    {InverterRL_STD(11), CAN_ID_STD, INVERTER_RL_FaultCodes, {0}, 0, false},
    {InverterRL_STD(12), CAN_ID_STD, INVERTER_RL_TorqueTimerParameters, {0}, 0, false},
    {InverterRL_STD(13), CAN_ID_STD, INVERTER_RL_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRL_STD(14), CAN_ID_STD, INVERTER_RL_FirmwareInformation, {0}, 0, false},
    {InverterRL_STD(15), CAN_ID_STD, INVERTER_RL_DiagnosticData, {0}, 0, false},
    {InverterRL_STD(16), CAN_ID_STD, INVERTER_RL_HighSpeedParameters, {0}, 0, false},
    {InverterRL_STD(17), CAN_ID_STD, INVERTER_RL_TorqueCapability, {0}, 0, false},

    // Inverter 2 (Standard 11-bit)
    {InverterRR_STD(0), CAN_ID_STD, INVERTER_RR_Temp1, {0}, 0, false},
    {InverterRR_STD(1), CAN_ID_STD, INVERTER_RR_Temp2, {0}, 0, false},
    {InverterRR_STD(2), CAN_ID_STD, INVERTER_RR_Temp3TorqueShudder, {0}, 0, false},
    {InverterRR_STD(3), CAN_ID_STD, INVERTER_RR_AnalogInputStatus, {0}, 0, false},
    {InverterRR_STD(4), CAN_ID_STD, INVERTER_RR_DigitalInputStatus, {0}, 0, false},
    {InverterRR_STD(5), CAN_ID_STD, INVERTER_RR_MotorPositionParameters, {0}, 0, false},
    {InverterRR_STD(6), CAN_ID_STD, INVERTER_RR_CurrentParameters, {0}, 0, false},
    {InverterRR_STD(7), CAN_ID_STD, INVERTER_RR_VoltageParameters, {0}, 0, false},
    {InverterRR_STD(8), CAN_ID_STD, INVERTER_RR_FluxParameters, {0}, 0, false},
    {InverterRR_STD(9), CAN_ID_STD, INVERTER_RR_InternalVoltageParameters, {0}, 0, false},
    {InverterRR_STD(10), CAN_ID_STD, INVERTER_RR_InternalStateParameters, {0}, 0, false},
    {InverterRR_STD(11), CAN_ID_STD, INVERTER_RR_FaultCodes, {0}, 0, false},
    {InverterRR_STD(12), CAN_ID_STD, INVERTER_RR_TorqueTimerParameters, {0}, 0, false},
    {InverterRR_STD(13), CAN_ID_STD, INVERTER_RR_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRR_STD(14), CAN_ID_STD, INVERTER_RR_FirmwareInformation, {0}, 0, false},
    {InverterRR_STD(15), CAN_ID_STD, INVERTER_RR_DiagnosticData, {0}, 0, false},
    {InverterRR_STD(16), CAN_ID_STD, INVERTER_RR_HighSpeedParameters, {0}, 0, false},
    {InverterRR_STD(17), CAN_ID_STD, INVERTER_RR_TorqueCapability, {0}, 0, false},

    // Inverter 1 (29-bit)
    {InverterRL_EXT(0), CAN_ID_EXT, INVERTER_RL_Temp1, {0}, 0, false},
    {InverterRL_EXT(1), CAN_ID_EXT, INVERTER_RL_Temp2, {0}, 0, false},
    {InverterRL_EXT(2), CAN_ID_EXT, INVERTER_RL_Temp3TorqueShudder, {0}, 0, false},
    {InverterRL_EXT(3), CAN_ID_EXT, INVERTER_RL_AnalogInputStatus, {0}, 0, false},
    {InverterRL_EXT(4), CAN_ID_EXT, INVERTER_RL_DigitalInputStatus, {0}, 0, false},
    {InverterRL_EXT(5), CAN_ID_EXT, INVERTER_RL_MotorPositionParameters, {0}, 0, false},
    {InverterRL_EXT(6), CAN_ID_EXT, INVERTER_RL_CurrentParameters, {0}, 0, false},
    {InverterRL_EXT(7), CAN_ID_EXT, INVERTER_RL_VoltageParameters, {0}, 0, false},
    {InverterRL_EXT(8), CAN_ID_EXT, INVERTER_RL_FluxParameters, {0}, 0, false},
    {InverterRL_EXT(9), CAN_ID_EXT, INVERTER_RL_InternalVoltageParameters, {0}, 0, false},
    {InverterRL_EXT(10), CAN_ID_EXT, INVERTER_RL_InternalStateParameters, {0}, 0, false},
    {InverterRL_EXT(11), CAN_ID_EXT, INVERTER_RL_FaultCodes, {0}, 0, false},
    {InverterRL_EXT(12), CAN_ID_EXT, INVERTER_RL_TorqueTimerParameters, {0}, 0, false},
    {InverterRL_EXT(13), CAN_ID_EXT, INVERTER_RL_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRL_EXT(14), CAN_ID_EXT, INVERTER_RL_FirmwareInformation, {0}, 0, false},
    {InverterRL_EXT(15), CAN_ID_EXT, INVERTER_RL_DiagnosticData, {0}, 0, false},
    {InverterRL_EXT(16), CAN_ID_EXT, INVERTER_RL_HighSpeedParameters, {0}, 0, false},
    {InverterRL_EXT(17), CAN_ID_EXT, INVERTER_RL_TorqueCapability, {0}, 0, false},

    // Inverter 2 (29-bit)
    {InverterRR_EXT(0), CAN_ID_EXT, INVERTER_RR_Temp1, {0}, 0, false},
    {InverterRR_EXT(1), CAN_ID_EXT, INVERTER_RR_Temp2, {0}, 0, false},
    {InverterRR_EXT(2), CAN_ID_EXT, INVERTER_RR_Temp3TorqueShudder, {0}, 0, false},
    {InverterRR_EXT(3), CAN_ID_EXT, INVERTER_RR_AnalogInputStatus, {0}, 0, false},
    {InverterRR_EXT(4), CAN_ID_EXT, INVERTER_RR_DigitalInputStatus, {0}, 0, false},
    {InverterRR_EXT(5), CAN_ID_EXT, INVERTER_RR_MotorPositionParameters, {0}, 0, false},
    {InverterRR_EXT(6), CAN_ID_EXT, INVERTER_RR_CurrentParameters, {0}, 0, false},
    {InverterRR_EXT(7), CAN_ID_EXT, INVERTER_RR_VoltageParameters, {0}, 0, false},
    {InverterRR_EXT(8), CAN_ID_EXT, INVERTER_RR_FluxParameters, {0}, 0, false},
    {InverterRR_EXT(9), CAN_ID_EXT, INVERTER_RR_InternalVoltageParameters, {0}, 0, false},
    {InverterRR_EXT(10), CAN_ID_EXT, INVERTER_RR_InternalStateParameters, {0}, 0, false},
    {InverterRR_EXT(11), CAN_ID_EXT, INVERTER_RR_FaultCodes, {0}, 0, false},
    {InverterRR_EXT(12), CAN_ID_EXT, INVERTER_RR_TorqueTimerParameters, {0}, 0, false},
    {InverterRR_EXT(13), CAN_ID_EXT, INVERTER_RR_ModulationIndexFluxWeakening, {0}, 0, false},
    {InverterRR_EXT(14), CAN_ID_EXT, INVERTER_RR_FirmwareInformation, {0}, 0, false},
    {InverterRR_EXT(15), CAN_ID_EXT, INVERTER_RR_DiagnosticData, {0}, 0, false},
    {InverterRR_EXT(16), CAN_ID_EXT, INVERTER_RR_HighSpeedParameters, {0}, 0, false},
    {InverterRR_EXT(17), CAN_ID_EXT, INVERTER_RR_TorqueCapability, {0}, 0, false},
};
// Must be after the canData array definition
const size_t canDataLength = sizeof(canData) / sizeof(CAN_Data_Entry);