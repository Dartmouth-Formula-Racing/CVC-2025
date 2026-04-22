/*
 * core_can.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef CVC_CAN_H
#define CVC_CAN_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stm32f4xx_hal_can.h>

#define CAN_BUFFER_LENGTH 64
#define CAN_MAX_SEND_TIME 10  // ms
#define CAN_TX_QUEUE_TIMEOUT_MS 0

#define CAN_EMUS_USE_EXT 0        // 1 if using extended IDs, 0 if using standard IDs
#define CAN_EMUS_BASE_EXT 0x19B5  // Base ID for EMUS BMS 29-bit IDs
#define CAN_EMUS_BASE_STD 0x320   // Base ID for EMUS BMS 11-bit IDs

#define CAN_VDM_USE_EXT 1         // 1 if using extended IDs, 0 if using standard IDs
#define CAN_VDM_BASE_EXT 0x0000A  // Base ID for VDM 29-bit IDs

#define CAN_INVERTER_USE_EXT 0           // 1 if using extended IDs, 0 if using standard IDs
#define CAN_INVERTER_BASE_ID1_STD 0x0D0  // Base ID for Inverter 1
#define CAN_INVERTER_BASE_ID2_STD 0x0A0  // Base ID for Inverter 2
#define CAN_INVERTER_BASE_ID1_EXT 0x6C0  // Base ID for Inverter 1
#define CAN_INVERTER_BASE_ID2_EXT 0x5C0  // Base ID for Inverter 2

#define CAN_DASHBOARD_USE_EXT 0
#define CAN_DASHBOARD_BASE_STD 0x750

#define CAN_BROADCAST_USE_EXT 0
#define CAN_BROADCAST_BASE_STD CAN_DASHBOARD_BASE_STD

#define CAN_SAFETYBROADCAST_INTERVAL 50  // ms
#define CAN_BROADCAST_INTERVAL 33        // ms

#define EMUS_STD(idx) (CAN_EMUS_BASE_STD + (idx))
#define EMUS_EXT(idx) ((CAN_EMUS_BASE_EXT << 16) | (idx))
#define VDM_EXT(idx) ((CAN_VDM_BASE_EXT << 16) | (idx))
#define InverterRL_STD(idx) (CAN_INVERTER_BASE_ID1_STD + (idx))
#define InverterRR_STD(idx) (CAN_INVERTER_BASE_ID2_STD + (idx))
#define InverterRL_EXT(idx) (CAN_INVERTER_BASE_ID1_EXT + (idx))
#define InverterRR_EXT(idx) (CAN_INVERTER_BASE_ID2_EXT + (idx))
#define Dashboard_STD(idx) (CAN_DASHBOARD_BASE_STD + (idx))

extern SemaphoreHandle_t canMutex;

typedef enum {
    EMUS_OverallParameters,
    EMUS_DiagnosticCodes,
    EMUS_BatteryVoltageOverallParameters,
    EMUS_CellModuleTemperatureOverallParameters,
    EMUS_CellTemperatureOverallParameters,
    EMUS_CellBalancingRateOverallParameters,
    EMUS_StateOfChargeParameters,
    EMUS_ConfigurationParameters,
    EMUS_ContactorControl,
    EMUS_EnergyParameters,
    EMUS_Events,
    VDM_GPSLatitudeLongitude,
    VDM_GPSData,
    VDM_GPSDateTime,
    VDM_AccelerationData,
    VDM_YawRateData,
    INVERTER_RL_Temp1,
    INVERTER_RL_Temp2,
    INVERTER_RL_Temp3TorqueShudder,
    INVERTER_RL_AnalogInputStatus,
    INVERTER_RL_DigitalInputStatus,
    INVERTER_RL_MotorPositionParameters,
    INVERTER_RL_CurrentParameters,
    INVERTER_RL_VoltageParameters,
    INVERTER_RL_FluxParameters,
    INVERTER_RL_InternalVoltageParameters,
    INVERTER_RL_InternalStateParameters,
    INVERTER_RL_FaultCodes,
    INVERTER_RL_TorqueTimerParameters,
    INVERTER_RL_ModulationIndexFluxWeakening,
    INVERTER_RL_FirmwareInformation,
    INVERTER_RL_DiagnosticData,
    INVERTER_RL_HighSpeedParameters,
    INVERTER_RL_TorqueCapability,
    INVERTER_RR_Temp1,
    INVERTER_RR_Temp2,
    INVERTER_RR_Temp3TorqueShudder,
    INVERTER_RR_AnalogInputStatus,
    INVERTER_RR_DigitalInputStatus,
    INVERTER_RR_MotorPositionParameters,
    INVERTER_RR_CurrentParameters,
    INVERTER_RR_VoltageParameters,
    INVERTER_RR_FluxParameters,
    INVERTER_RR_InternalVoltageParameters,
    INVERTER_RR_InternalStateParameters,
    INVERTER_RR_FaultCodes,
    INVERTER_RR_TorqueTimerParameters,
    INVERTER_RR_ModulationIndexFluxWeakening,
    INVERTER_RR_FirmwareInformation,
    INVERTER_RR_DiagnosticData,
    INVERTER_RR_HighSpeedParameters,
    INVERTER_RR_TorqueCapability,
    // === Length of CVC data array ===
    // This must be the last value in the enum
    NUM_MESSAGES,
} CAN_Message_Index;

typedef struct {
    uint32_t canID;
    uint32_t canIDE;
    CAN_Message_Index index;
    uint8_t data[8];
    uint32_t timestamp;
    bool parsed;
} CAN_Data_Entry;

typedef union {
    CAN_RxHeaderTypeDef rx;
    CAN_TxHeaderTypeDef tx;
} CAN_Header;

typedef struct {
    CAN_Header header;
    uint8_t data[8];
} CAN_Frame;

extern CAN_Data_Entry canData[];
extern const size_t canDataLength;

typedef enum {
    BUS1,
    BUS2,
} CAN_Bus;

void CAN_Init(void);

CAN_Data_Entry CAN_getDataByIndex(CAN_Message_Index index);
CAN_Data_Entry* CAN_getPtrByIndex(CAN_Message_Index index);

CAN_Data_Entry CAN_getDataByID(uint32_t canID, uint32_t canIDE);
CAN_Data_Entry* CAN_getPtrByID(uint32_t canID, uint32_t canIDE);

void CAN_SendFrame(CAN_Bus bus, CAN_Frame* frame);

uint32_t CAN_GetBus1TxDropCount(void);
uint32_t CAN_GetBus2TxDropCount(void);
uint32_t CAN_GetRxDropCount(void);

#endif  // CVC_CAN_H