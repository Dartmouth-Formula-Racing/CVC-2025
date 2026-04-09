/*
 * data.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef CVC_DATA_H
#define CVC_DATA_H

#include <FreeRTOS.h>
#include <semphr.h>

typedef enum {
    CVC_EFFICIENCY,         // Vehicle efficiency (W/km)
    CVC_ODOMETER,           // Odometer (m)
    CVC_DRIVE_MODE,         // 0 = neutral, 1 = drive, 2 = reverse
    CVC_STATE,              // vehicle_state_t (state machine state)
    CVC_LV_VOLTAGE,         // Low voltage system voltage
    CVC_LEFT_TORQUE,        // Left motor torque
    CVC_RIGHT_TORQUE,       // Right motor torque
    CVC_LEFT_DIRECTION,     // Left motor direction
    CVC_RIGHT_DIRECTION,    // Right motor direction
    CVC_PRECHARGE_BTN,      // Precharge button state
    CVC_LV_CHARGE_STATE,    // LV charge state
    CVC_AIR_1_STATE,        // Air 1 state
    CVC_AIR_2_STATE,        // Air 2 state
    CVC_DCDC_STATE,         // DCDC state
    CVC_COCKPIT_BRB_STATE,  // Cockpit BRB state
    CVC_BOT_STATE,          // BOT state
    CVC_IMD_STATE,          // IMD state
    CVC_BMS_STATE,          // BMS state
    CVC_MAIN_LOOP_TIME,     // Main loop time (ms)
    CVC_DATA_LENGTH,
} CVC_Data_Index;

typedef enum {
    BMS_IGNITION,
    BMS_CHARGER_MAINS,
    BMS_FAST_CHARGE,
    BMS_LEAKAGE_DETECTED,
    BMS_CHARGER_ENABLED,
    BMS_HEATER_ENABLED,
    BMS_BATTERY_CONTACTOR,
    BMS_BATTERY_FAN,
    BMS_POWER_REDUCTION,
    BMS_CHARGING_INTERLOCK,
    BMS_DCDC_ENABLED,
    BMS_CONTACTOR_PRECHARGE,
    BMS_CHARGING_STATE,
    BMS_CHARGING_DURATION,
    BMS_LAST_CHARGING_ERROR,
    BMS_LIVE_CELL_COUNT,
    // Diagnostic Codes
    BMS_UNDERVOLTAGE,
    BMS_OVERVOLTAGE,
    BMS_DISCHARGE_OVERCURRENT,
    BMS_CHARGE_OVERCURRENT,
    BMS_CELL_MODULE_OVERHEAT,
    BMS_LEAKAGE,
    BMS_NO_CELL_COMMUNICATION,
    BMS_WARN_LOW_CELL_VOLTAGE,
    BMS_WARN_HIGH_DISCHARGE_CURRENT,
    BMS_WARN_HIGH_CELL_MODULE_TEMP,
    BMS_CELL_OVERHEAT,
    BMS_NO_CURRENT_SENSOR,
    BMS_PACK_UNDERVOLTAGE,
    BMS_CELL_VOLTAGE_VALID,
    BMS_CELL_MODULE_TEMP_VALID,
    BMS_BALANCE_RATE_VALID,
    BMS_LIVE_CELL_COUNT_VALID,
    BMS_CHARGING_FINISHED,
    BMS_CELL_TEMP_VALID,
    // Battery Voltage Overall Parameters
    BMS_MIN_CELL_VOLTAGE,
    BMS_MAX_CELL_VOLTAGE,
    BMS_AVG_CELL_VOLTAGE,
    BMS_TOTAL_VOLTAGE,
    // Cell Module Temperature Overall Parameters
    BMS_MIN_CELL_MODULE_TEMP,
    BMS_MAX_CELL_MODULE_TEMP,
    BMS_AVG_CELL_MODULE_TEMP,
    // Cell Module Temperature Overall Parameters
    BMS_MIN_CELL_TEMP,
    BMS_MAX_CELL_TEMP,
    BMS_AVG_CELL_TEMP,
    // Cell Balancing Rate Overall Parameters
    BMS_MIN_BALANCE_RATE,
    BMS_MAX_BALANCE_RATE,
    BMS_AVG_BALANCE_RATE,
    // State of Charge Parameters
    BMS_CURRENT,
    BMS_ESTIMATED_CHARGE,
    BMS_ESTIMATED_SOC,
    BMS_ESTIMATED_SOH,
    // Configuration Parameters - TODO: Figure out how to handle this
    // Contactor Control
    BMS_CONTACTOR_STATE,
    // Energy Parameters
    BMS_ESTIMATED_CONSUMPTION,
    BMS_ESTIMATED_ENERGY,
    BMS_ESTIMATED_DISTANCE_REMAINING,
    BMS_DISTANCE_TRAVELED,
    BMS_DATA_LENGTH,
} BMS_Data_Index;

typedef enum {
    VDM_GPS_LATITUDE,
    VDM_GPS_LONGITUDE,
    VDM_GPS_SPEED,
    VDM_GPS_ALTITUDE,
    VDM_GPS_TRUE_COURSE,
    VDM_GPS_SATELLITES_IN_USE,
    VDM_GPS_DATA_VALID,
    // GPS Date Time Parameters
    VDM_GPS_DATE_TIME_VALID,
    VDM_UTC_YEAR,
    VDM_UTC_MONTH,
    VDM_UTC_DAY,
    VDM_UTC_HOUR,
    VDM_UTC_MINUTE,
    VDM_UTC_SECOND,
    // Acceleration data
    VDM_ACCELERATION_X,
    VDM_ACCELERATION_Y,
    VDM_ACCELERATION_Z,
    // Yaw rate Data
    VDM_YAW_RATE_X,
    VDM_YAW_RATE_Y,
    VDM_YAW_RATE_Z,
    VDM_DATA_LENGTH,
} VDM_Data_Index;

typedef enum {
    INVERTER_POWER_MODULE_A_TEMP,
    INVERTER_POWER_MODULE_B_TEMP,
    INVERTER_POWER_MODULE_C_TEMP,
    INVERTER_GATE_DRIVER_BOARD_TEMP,
    INVERTER_CONTROL_BOARD_TEMP,
    INVERTER_RTD_INPUT_1_TEMP,
    INVERTER_RTD_INPUT_2_TEMP,
    INVERTER_STALL_BURST_MODEL_TEMP,
    INVERTER_COOLANT_TEMP,
    INVERTER_HOT_SPOT_TEMP,
    INVERTER_MOTOR_TEMP,
    // Torque Parameters
    INVERTER_TORQUE_SHUDDER,
    // Analog Input Status Parameters
    INVERTER_ANALOG_INPUT_1,
    INVERTER_ANALOG_INPUT_2,
    INVERTER_ANALOG_INPUT_3,
    INVERTER_ANALOG_INPUT_4,
    INVERTER_ANALOG_INPUT_5,
    INVERTER_ANALOG_INPUT_6,
    // Digital Input Status Parameters
    INVERTER_FORWARD_SWITCH,
    INVERTER_REVERSE_SWITCH,
    INVERTER_BRAKE_SWITCH,
    INVERTER_REGEN_DISABLE_SWITCH,
    INVERTER_IGNITION_SWITCH,
    INVERTER_START_SWITCH,
    INVERTER_VALET_MODE,
    // Motor Position Parameters
    INVERTER_MOTOR_ANGLE,
    INVERTER_MOTOR_SPEED,
    INVERTER_MOTOR_ACCELERATION,
    INVERTER_ELECTRICAL_OUTPUT_FREQUENCY,
    INVERTER_DELTA_RESOLVER_FILTERED,
    // Current Parameters
    INVERTER_PHASE_A_CURRENT,
    INVERTER_PHASE_B_CURRENT,
    INVERTER_PHASE_C_CURRENT,
    INVERTER_DC_BUS_CURRENT,
    // Voltage Parameters
    INVERTER_DC_BUS_VOLTAGE,
    INVERTER_OUTPUT_VOLTAGE,
    INVERTER_VAB_VD_VOLTAGE,
    INVERTER_VAB_VQ_VOLTAGE,
    // Flux Parameters
    INVERTER_FLUX_COMMAND,
    INVERTER_FLUX_FEEDBACK,
    INVERTER_ID_CURRENT,
    INVERTER_IQ_CURRENT,
    // Internal Voltage Parameters
    INVERTER_1_5_REFERENCE_VOLTAGE,
    INVERTER_2_5_REFERENCE_VOLTAGE,
    INVERTER_5_0_REFERENCE_VOLTAGE,
    INVERTER_12_0_REFERENCE_VOLTAGE,
    // Internal State Parameters
    INVERTER_VSM_STATE,
    INVERTER_PWM_FREQUENCY,
    INVERTER_STATE,
    INVERTER_RELAY_1_STATUS,
    INVERTER_RELAY_2_STATUS,
    INVERTER_RELAY_3_STATUS,
    INVERTER_RELAY_4_STATUS,
    INVERTER_RELAY_5_STATUS,
    INVERTER_RELAY_6_STATUS,
    INVERTER_RUN_MODE,
    INVERTER_SELF_SENSING_ASSIST,
    INVERTER_ACTIVE_DISCHARGE_STATE,
    INVERTER_COMMAND_MODE,
    INVERTER_ROLLING_COUNTER_VALUE,
    INVERTER_ENABLE_STATE,
    INVERTER_BURST_MODEL_MODE,
    INVERTER_START_MODE_ACTIVE,
    INVERTER_ENABLE_LOCKOUT,
    INVERTER_DIRECTION_COMMAND,
    INVERTER_BMS_ACTIVE,
    INVERTER_BMS_LIMITING_TORQUE,
    INVERTER_LIMIT_MAX_SPEED,
    INVERTER_LIMIT_HOT_SPOT,
    INVERTER_LOW_SPEED_LIMITING,
    INVERTER_COOLANT_TEMP_LIMITING,
    INVERTER_LIMIT_STALL_BURST_MODEL,
    // Fault codes
    INVERTER_POST_FAULT_LO,
    INVERTER_POST_FAULT_HI,
    INVERTER_RUN_FAULT_LO,
    INVERTER_RUN_FAULT_HI,
    // High Speed Parameters
    INVERTER_TORQUE_COMMAND_HS,
    INVERTER_TORQUE_FEEDBACK_HS,
    INVERTER_MOTOR_SPEED_HS,
    INVERTER_DC_BUS_VOLTAGE_HS,
    INVERTER_DATA_LENGTH,
} Inverter_Data_Index;

typedef struct {
    uint32_t data;
    TickType_t time;
} DataEntry;

extern DataEntry CVCData[CVC_DATA_LENGTH];
extern DataEntry BMSData[BMS_DATA_LENGTH];
extern DataEntry VDMData[VDM_DATA_LENGTH];
extern DataEntry InverterRLData[INVERTER_DATA_LENGTH];
extern DataEntry InverterRRData[INVERTER_DATA_LENGTH];

extern SemaphoreHandle_t CVCDataMutex;
extern SemaphoreHandle_t BMSDataMutex;
extern SemaphoreHandle_t VDMDataMutex;
extern SemaphoreHandle_t InverterRLDataMutex;
extern SemaphoreHandle_t InverterRRDataMutex;

void Data_Init(void);
void CVCDataSet(CVC_Data_Index index, uint32_t value);
void BMSDataSet(BMS_Data_Index index, uint32_t value);
void VDMDataSet(VDM_Data_Index index, uint32_t value);
void InverterRLDataSet(Inverter_Data_Index index, uint32_t value);
void InverterRRDataSet(Inverter_Data_Index index, uint32_t value);
DataEntry CVCDataGet(CVC_Data_Index index);
DataEntry BMSDataGet(BMS_Data_Index index);
DataEntry VDMDataGet(VDM_Data_Index index);
DataEntry InverterRLDataGet(Inverter_Data_Index index);
DataEntry InverterRRDataGet(Inverter_Data_Index index);

#endif  // CVC_DATA_H