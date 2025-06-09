/*
 * parse.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#include <can.h>
#include <data.h>
#include <parse.h>
#include <string.h>

// TODO: Implement 11-bit CAN message parsing functions
// Only 29-bit CAN message parsing functions are currently implemented
// ========== EMUS BMS Parsing Functions ==========

/**
 * @brief Parses EMUS BMS 29-bit Overall Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_OverallParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_OverallParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }

    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_OverallParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);

    // Byte 0: Input signals
    // Bit 0: Ignition key
    // Bit 1: Charger mains
    // Bit 2: Fast charge
    // Bit 3: Leakage sensor
    // Bits 4-7: Don't care
    BMSData[BMS_IGNITION].data = (data[0] >> 0) & 0x01;
    BMSData[BMS_CHARGER_MAINS].data = (data[0] >> 1) & 0x01;
    BMSData[BMS_FAST_CHARGE].data = (data[0] >> 2) & 0x01;
    BMSData[BMS_LEAKAGE_DETECTED].data = (data[0] >> 3) & 0x01;
    // Byte 1: Output signals
    // Bit 0: Charger enable
    // Bit 1: Heater enable
    // Bit 2: Battery contactor
    // Bit 3: Battery fan
    // Bit 4: Power reduction
    // Bit 5: Charging interlock
    // Bit 6: DCDC control
    // Bit 7: Contactor precharge
    BMSData[BMS_CHARGER_ENABLED].data = (data[1] >> 0) & 0x01;
    BMSData[BMS_HEATER_ENABLED].data = (data[1] >> 1) & 0x01;
    BMSData[BMS_BATTERY_CONTACTOR].data = (data[1] >> 2) & 0x01;
    BMSData[BMS_BATTERY_FAN].data = (data[1] >> 3) & 0x01;
    BMSData[BMS_POWER_REDUCTION].data = (data[1] >> 4) & 0x01;
    BMSData[BMS_CHARGING_INTERLOCK].data = (data[1] >> 5) & 0x01;
    BMSData[BMS_DCDC_ENABLED].data = (data[1] >> 6) & 0x01;
    BMSData[BMS_CONTACTOR_PRECHARGE].data = (data[1] >> 7) & 0x01;
    // Byte 2: Number of live cells (MSB)
    // Byte 3: Charging state
    // 0 = Disconnected
    // 1 = Pre-heating
    // 2 = Pre-charging
    // 3 = Main charging
    // 4 = Balancing
    // 5 = Charging finished
    // 6 = Charging error
    BMSData[BMS_CHARGING_STATE].data = data[3];
    // Byte 4: Charging stage duration (MSB)
    // Byte 5: Charging stage duration (LSB)
    BMSData[BMS_CHARGING_DURATION].data = (data[4] << 8) | data[5];
    // Charging stage duration is in minutes
    // Byte 6: Last charging error
    // 0 = No error
    // 1 = No cell communication at start of charging or communication lost during pre-charging (using CAN charger)
    // 2 = No cell communication using non-CAN charger
    // 3 = Maximum chargin stage duration exceeded
    // 4 = Cell communication lost during charging (CAN charger)
    // 5 = Cannot set cell module balancing threshold
    // 6 = Cell or cell module temperature too high
    // 7 = Cell commmunication lost during pre-heating (CAN charger)
    BMSData[BMS_LAST_CHARGING_ERROR].data = data[6];
    // Byte 7: Number of live cells (LSB)
    BMSData[BMS_LIVE_CELL_COUNT].data = (data[2] << 8) | data[7];

    canData[EMUS_OverallParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Diagnostic Codes CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_DiagnosticCodes() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_DiagnosticCodes].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_DiagnosticCodes].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Protection flags (LSB)
    // Bit 0: Cell undervoltage
    // Bit 1: Cell overvoltage
    // Bit 2: Discharge overcurrent
    // Bit 3: Charge overcurrent
    // Bit 4: Cell module overheat
    // Bit 5: Leakage
    // Bit 6: No cell communication
    // Bit 7: Don't care
    BMSData[BMS_UNDERVOLTAGE].data = (data[0] >> 0) & 0x01;
    BMSData[BMS_OVERVOLTAGE].data = (data[0] >> 1) & 0x01;
    BMSData[BMS_DISCHARGE_OVERCURRENT].data = (data[0] >> 2) & 0x01;
    BMSData[BMS_CHARGE_OVERCURRENT].data = (data[0] >> 3) & 0x01;
    BMSData[BMS_CELL_MODULE_OVERHEAT].data = (data[0] >> 4) & 0x01;
    BMSData[BMS_LEAKAGE].data = (data[0] >> 5) & 0x01;
    BMSData[BMS_NO_CELL_COMMUNICATION].data = (data[0] >> 6) & 0x01;
    // Byte 1: Warning (power reduction) flags
    // Bit 0: Low cell voltage
    // Bit 1: High discharge current
    // Bit 2: High cell module temperature
    BMSData[BMS_WARN_LOW_CELL_VOLTAGE].data = (data[1] >> 0) & 0x01;
    BMSData[BMS_WARN_HIGH_DISCHARGE_CURRENT].data = (data[1] >> 1) & 0x01;
    BMSData[BMS_WARN_HIGH_CELL_MODULE_TEMP].data = (data[1] >> 2) & 0x01;
    // Byte 3: Protection flags (MSB)
    // Bit 3: Cell overheat
    // Bit 4: No current sensor
    // Bit 5: Pack undervoltage
    BMSData[BMS_CELL_OVERHEAT].data = (data[3] >> 3) & 0x01;
    BMSData[BMS_NO_CURRENT_SENSOR].data = (data[3] >> 4) & 0x01;
    BMSData[BMS_PACK_UNDERVOLTAGE].data = (data[3] >> 5) & 0x01;
    // Byte 4: Battery status flags
    // Bit 0: Cell voltages valid
    // Bit 1: Cell module temperatures valid
    // Bit 2: Cell balancing rates valid
    // Bit 3: Cell balancing thresholds valid
    // Bit 4: Charging finished
    // Bit 5: Cell temperatures valid
    BMSData[BMS_CELL_VOLTAGE_VALID].data = (data[4] >> 0) & 0x01;
    BMSData[BMS_CELL_MODULE_TEMP_VALID].data = (data[4] >> 1) & 0x01;
    BMSData[BMS_BALANCE_RATE_VALID].data = (data[4] >> 2) & 0x01;
    BMSData[BMS_LIVE_CELL_COUNT_VALID].data = (data[4] >> 3) & 0x01;
    BMSData[BMS_CHARGING_FINISHED].data = (data[4] >> 4) & 0x01;
    BMSData[BMS_CELL_TEMP_VALID].data = (data[4] >> 5) & 0x01;

    canData[EMUS_DiagnosticCodes].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Battery Voltage Overall Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_BatteryVoltageOverallParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_BatteryVoltageOverallParameters].parsed) {
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_BatteryVoltageOverallParameters].data, sizeof(data));
    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);

    // Byte 0: Min cell voltage
    BMSData[BMS_MIN_CELL_VOLTAGE].data = data[0] + 200;
    // Byte 1: Max cell voltage
    BMSData[BMS_MAX_CELL_VOLTAGE].data = data[1] + 200;
    // Byte 2: Average cell voltage
    BMSData[BMS_AVG_CELL_VOLTAGE].data = data[2] + 200;
    // Byte 4: Total voltage (LSB)
    BMSData[BMS_TOTAL_VOLTAGE].data = data[4] & 0xFF;
    // Byte 3: Total voltage (2nd byte)
    BMSData[BMS_TOTAL_VOLTAGE].data = data[3] << 8 | BMSData[BMS_TOTAL_VOLTAGE].data;
    // Byte 6: Total voltage (3rd byte)
    BMSData[BMS_TOTAL_VOLTAGE].data = data[6] << 16 | BMSData[BMS_TOTAL_VOLTAGE].data;
    // Byte 5: Total voltage (MSB)
    BMSData[BMS_TOTAL_VOLTAGE].data = data[5] << 24 | BMSData[BMS_TOTAL_VOLTAGE].data;
    // Byte 7: Don't care

    canData[EMUS_BatteryVoltageOverallParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Cell Module Temperature Overall Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_CellModuleTemperatureOverallParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_CellModuleTemperatureOverallParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_CellModuleTemperatureOverallParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Min cell module temperature
    BMSData[BMS_MIN_CELL_MODULE_TEMP].data = data[0];
    // Byte 1: Max cell module temperature
    BMSData[BMS_MAX_CELL_MODULE_TEMP].data = data[1];
    // Byte 2: Average cell module temperature
    BMSData[BMS_AVG_CELL_MODULE_TEMP].data = data[2];
    // Byte 3-7: Don't care

    canData[EMUS_CellModuleTemperatureOverallParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Cell Temperature Overall Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_CellTemperatureOverallParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_CellTemperatureOverallParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_CellTemperatureOverallParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Min cell temperature
    BMSData[BMS_MIN_CELL_TEMP].data = data[0];
    // Byte 1: Max cell temperature
    BMSData[BMS_MAX_CELL_TEMP].data = data[1];
    // Byte 2: Average cell temperature
    BMSData[BMS_AVG_CELL_TEMP].data = data[2];
    // Byte 3-7: Don't care

    canData[EMUS_CellTemperatureOverallParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Cell Balancing Rate Overall Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_CellBalancingRateOverallParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_CellBalancingRateOverallParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_CellBalancingRateOverallParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Min cell balancing rate
    BMSData[BMS_MIN_BALANCE_RATE].data = data[0];
    // Byte 1: Max cell balancing rate
    BMSData[BMS_MAX_BALANCE_RATE].data = data[1];
    // Byte 2: Average cell balancing rate
    BMSData[BMS_AVG_BALANCE_RATE].data = data[2];
    // Byte 3-7: Don't care

    canData[EMUS_CellBalancingRateOverallParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit State of Charge Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_StateOfChargeParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_StateOfChargeParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_StateOfChargeParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Current (MSB)
    // Byte 1: Current (LSB)
    BMSData[BMS_CURRENT].data = (data[0] << 8) | data[1];
    // Byte 2: Estimated charge (MSB)
    // Byte 3: Estimated charge (LSB)
    BMSData[BMS_ESTIMATED_CHARGE].data = (data[2] << 8) | data[3];
    // Byte 4: Don't care
    // Byte 5: Estimated user state of charge (MSB)
    // Byte 6: Estimated user state of charge (LSB)
    BMSData[BMS_ESTIMATED_SOC].data = (data[5] << 8) | data[6];
    // Byte 7: Estimated state of health
    BMSData[BMS_ESTIMATED_SOH].data = data[7];

    canData[EMUS_StateOfChargeParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Configuration Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_ConfigurationParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_ConfigurationParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    // TODO: Implement
    canData[EMUS_ConfigurationParameters].parsed = true;
    xSemaphoreGive(canMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Contactor Control CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_ContactorControl() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_ContactorControl].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_ContactorControl].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Contactor state
    BMSData[BMS_CONTACTOR_STATE].data = data[0];
    // Byte 1-7: Don't care

    canData[EMUS_ContactorControl].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Energy Parameters CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_EnergyParameters() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_EnergyParameters].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[EMUS_EnergyParameters].data, sizeof(data));

    xSemaphoreTake(BMSDataMutex, portMAX_DELAY);
    // Byte 0: Estimated Consumption (MSB)
    // Byte 1: Estimated Consumption (LSB)
    BMSData[BMS_ESTIMATED_CONSUMPTION].data = (data[0] << 8) | data[1];
    // Byte 2: Estimated Energy (MSB)
    // Byte 3: Estimated Energy (LSB)
    BMSData[BMS_ESTIMATED_ENERGY].data = (data[2] << 8) | data[3];
    // Byte 4: Estimated Distance Remaining (MSB)
    // Byte 5: Estimated Distance Remaining (LSB)
    BMSData[BMS_ESTIMATED_DISTANCE_REMAINING].data = (data[4] << 8) | data[5];
    // Byte 6: Distance Traveled (MSB)
    // Byte 7: Distance Traveled (LSB)
    BMSData[BMS_DISTANCE_TRAVELED].data = (data[6] << 8) | data[7];

    canData[EMUS_EnergyParameters].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(BMSDataMutex);
}

/**
 * @brief Parses EMUS BMS 29-bit Events CAN message.
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_EMUS_Events() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[EMUS_Events].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    // TODO: Implement
    canData[EMUS_Events].parsed = true;
    xSemaphoreGive(canMutex);
}

/**
 * @brief Parses VDM GPS Latitude and Longitude 29-bit CAN message (0x0000A0000).
 * @param : A CAN frame containing 8 bytes of data to be parsed.
 * @retval None
 */
void CAN_Parse_VDM_GPSLatitudeLongitude() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[VDM_GPSLatitudeLongitude].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[VDM_GPSLatitudeLongitude].data, sizeof(data));

    xSemaphoreTake(VDMDataMutex, portMAX_DELAY);
    // Byte 0-3: GPS Latitude
    VDMData[VDM_GPS_LATITUDE].data = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    // Byte 4-7: GPS Longitude
    VDMData[VDM_GPS_LONGITUDE].data = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

    canData[VDM_GPSLatitudeLongitude].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(VDMDataMutex);
}

/**
 * @brief Parses VDM GPS data including speed, altitude, true course, satellites in use, and GPS validity (0x0000A0001).
 * @param : A CAN frame containing 8 bytes of data to be parsed.
 * @retval None
 */
void CAN_Parse_VDM_GPSData() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[VDM_GPSData].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[VDM_GPSData].data, sizeof(data));

    xSemaphoreTake(VDMDataMutex, portMAX_DELAY);
    // Byte 0,1: GPS Speed
    VDMData[VDM_GPS_SPEED].data = (data[0] << 8) | data[1];
    // Byte 2, 3: GPS Altitude
    VDMData[VDM_GPS_ALTITUDE].data = (data[2] << 8) | data[3];
    // Byte 4, 5: GPS True Course
    VDMData[VDM_GPS_TRUE_COURSE].data = (data[4] << 8) | data[5];
    // Byte 6: Satellites in use
    VDMData[VDM_GPS_SATELLITES_IN_USE].data = data[6];
    // Byte 7: GPS Validity
    VDMData[VDM_GPS_DATA_VALID].data = data[7];

    canData[VDM_GPSData].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(VDMDataMutex);
}

/**
 * @brief Parses VDM GPS date and time information (0x0000A0002).
 * @param : A CAN frame containing 8 bytes of data to be parsed.
 * @retval None
 */
void CAN_Parse_VDM_GPSDateTime() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[VDM_GPSDateTime].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[VDM_GPSDateTime].data, sizeof(data));

    xSemaphoreTake(VDMDataMutex, portMAX_DELAY);
    // Byte 0: GPS Validity
    VDMData[VDM_GPS_DATE_TIME_VALID].data = data[0];
    // Byte 1: UTC Year
    VDMData[VDM_UTC_YEAR].data = data[1];
    // Byte 2: UTC Month
    VDMData[VDM_UTC_MONTH].data = data[2];
    // Byte 3: UTC Day
    VDMData[VDM_UTC_DAY].data = data[3];
    // Byte 5: UTC Hour
    VDMData[VDM_UTC_HOUR].data = data[5];
    // Byte 6: UTC Minute
    VDMData[VDM_UTC_MINUTE].data = data[6];
    // Byte 7: UTC Second
    VDMData[VDM_UTC_SECOND].data = data[7];

    canData[VDM_GPSDateTime].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(VDMDataMutex);
}

/**
 * @brief Parses acceleration data for the X, Y, and Z axes (0x0000A0003).
 * @param : A CAN frame containing bytes of acceleration data.
 * @retval None
 */
void CAN_Parse_VDM_AccelerationData() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[VDM_AccelerationData].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[VDM_AccelerationData].data, sizeof(data));

    xSemaphoreTake(VDMDataMutex, portMAX_DELAY);
    // Byte 0, 1: X-Axis Acceleration
    VDMData[VDM_ACCELERATION_X].data = (data[0] << 8) | data[1];
    // Byte 2, 3: Y-Axis Acceleration
    VDMData[VDM_ACCELERATION_Y].data = (data[2] << 8) | data[3];
    // Byte 4, 5: Z-Axis Acceleration
    VDMData[VDM_ACCELERATION_Z].data = (data[4] << 8) | data[5];

    canData[VDM_AccelerationData].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(VDMDataMutex);
}

/**
 * @brief Parses yaw rate data for the X, Y, and Z axes (0x0000A0004).
 * @param : A CAN frame containing bytes of yaw rate data.
 * @retval None
 */
void CAN_Parse_VDM_YawRateData() {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (canData[VDM_YawRateData].parsed) {
        xSemaphoreGive(canMutex);
        return;
    }
    uint8_t data[8] = {0};
    memcpy(data, canData[VDM_YawRateData].data, sizeof(data));

    xSemaphoreTake(VDMDataMutex, portMAX_DELAY);
    // Byte 0, 1: X-Axis Yaw Rate
    VDMData[VDM_YAW_RATE_X].data = (data[0] << 8) | data[1];
    // Byte 2, 3: Y-Axis Yaw Rate
    VDMData[VDM_YAW_RATE_Y].data = (data[2] << 8) | data[3];
    // Byte 4, 5: Z-Axis Yaw Rate
    VDMData[VDM_YAW_RATE_Z].data = (data[4] << 8) | data[5];

    canData[VDM_YawRateData].parsed = true;

    xSemaphoreGive(canMutex);
    xSemaphoreGive(VDMDataMutex);
}

/**
 * @brief Parses Inverter 29-bit Temperatures #1 CAN message. (0)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_Temp1(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_Temp1].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_Temp1].data, sizeof(data));
        canData[INVERTER_RL_Temp1].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_POWER_MODULE_A_TEMP].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_POWER_MODULE_B_TEMP].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_POWER_MODULE_C_TEMP].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_GATE_DRIVER_BOARD_TEMP].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_Temp1].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_Temp1].data, sizeof(data));
        canData[INVERTER_RR_Temp1].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_POWER_MODULE_A_TEMP].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_POWER_MODULE_B_TEMP].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_POWER_MODULE_C_TEMP].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_GATE_DRIVER_BOARD_TEMP].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Temperatures #2 CAN message. (1)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_Temp2(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_Temp2].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_Temp2].data, sizeof(data));
        canData[INVERTER_RL_Temp2].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_CONTROL_BOARD_TEMP].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_RTD_INPUT_1_TEMP].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_RTD_INPUT_2_TEMP].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_STALL_BURST_MODEL_TEMP].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_Temp2].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_Temp2].data, sizeof(data));
        canData[INVERTER_RR_Temp2].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_CONTROL_BOARD_TEMP].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_RTD_INPUT_1_TEMP].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_RTD_INPUT_2_TEMP].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_STALL_BURST_MODEL_TEMP].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Temperatures #3 & Torque Shudder CAN message. (2)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_Temp3TorqueShudder(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_Temp3TorqueShudder].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_Temp3TorqueShudder].data, sizeof(data));
        canData[INVERTER_RL_Temp3TorqueShudder].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_COOLANT_TEMP].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_HOT_SPOT_TEMP].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_MOTOR_TEMP].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_TORQUE_SHUDDER].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_Temp3TorqueShudder].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_Temp3TorqueShudder].data, sizeof(data));
        canData[INVERTER_RR_Temp3TorqueShudder].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_COOLANT_TEMP].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_HOT_SPOT_TEMP].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_MOTOR_TEMP].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_TORQUE_SHUDDER].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Analog Input Status CAN message. (0x0A3)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
// 01234567 89012345 67890123 45678901 23456789 01234567 89012345 67890123
void CAN_Parse_Inverter_AnalogInputStatus(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    if (inverter == RL) {
        if (canData[INVERTER_RL_AnalogInputStatus].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        uint64_t data64 = 0;
        memcpy(&data64, canData[INVERTER_RL_AnalogInputStatus].data, sizeof(data64));
        canData[INVERTER_RL_AnalogInputStatus].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_ANALOG_INPUT_1].data = (data64 >> 0) & 0x3FF;
        InverterRLData[INVERTER_ANALOG_INPUT_2].data = (data64 >> 10) & 0x3FF;
        InverterRLData[INVERTER_ANALOG_INPUT_3].data = (data64 >> 20) & 0x3FF;
        InverterRLData[INVERTER_ANALOG_INPUT_4].data = (data64 >> 32) & 0x3FF;
        InverterRLData[INVERTER_ANALOG_INPUT_5].data = (data64 >> 42) & 0x3FF;
        InverterRLData[INVERTER_ANALOG_INPUT_6].data = (data64 >> 52) & 0x3FF;
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_AnalogInputStatus].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        uint64_t data64 = 0;
        memcpy(&data64, canData[INVERTER_RR_AnalogInputStatus].data, sizeof(data64));
        canData[INVERTER_RR_AnalogInputStatus].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_ANALOG_INPUT_1].data = (data64 >> 0) & 0x3FF;
        InverterRRData[INVERTER_ANALOG_INPUT_2].data = (data64 >> 10) & 0x3FF;
        InverterRRData[INVERTER_ANALOG_INPUT_3].data = (data64 >> 20) & 0x3FF;
        InverterRRData[INVERTER_ANALOG_INPUT_4].data = (data64 >> 32) & 0x3FF;
        InverterRRData[INVERTER_ANALOG_INPUT_5].data = (data64 >> 42) & 0x3FF;
        InverterRRData[INVERTER_ANALOG_INPUT_6].data = (data64 >> 52) & 0x3FF;
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Digital Input Status CAN message. (4)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_DigitalInputStatus(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_DigitalInputStatus].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_DigitalInputStatus].data, sizeof(data));
        canData[INVERTER_RL_DigitalInputStatus].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_FORWARD_SWITCH].data = data[1];
        InverterRLData[INVERTER_REVERSE_SWITCH].data = data[0];
        InverterRLData[INVERTER_BRAKE_SWITCH].data = data[3];
        InverterRLData[INVERTER_REGEN_DISABLE_SWITCH].data = data[2];
        InverterRLData[INVERTER_IGNITION_SWITCH].data = data[5];
        InverterRLData[INVERTER_START_SWITCH].data = data[4];
        InverterRLData[INVERTER_VALET_MODE].data = data[7];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_DigitalInputStatus].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_DigitalInputStatus].data, sizeof(data));
        canData[INVERTER_RR_DigitalInputStatus].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_FORWARD_SWITCH].data = data[1];
        InverterRRData[INVERTER_REVERSE_SWITCH].data = data[0];
        InverterRRData[INVERTER_BRAKE_SWITCH].data = data[3];
        InverterRRData[INVERTER_REGEN_DISABLE_SWITCH].data = data[2];
        InverterRRData[INVERTER_IGNITION_SWITCH].data = data[5];
        InverterRRData[INVERTER_START_SWITCH].data = data[4];
        InverterRRData[INVERTER_VALET_MODE].data = data[7];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Motor Position Parameters CAN message. (5)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_MotorPositionParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_MotorPositionParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_MotorPositionParameters].data, sizeof(data));
        canData[INVERTER_RL_MotorPositionParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_MOTOR_ANGLE].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_MOTOR_SPEED].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_ELECTRICAL_OUTPUT_FREQUENCY].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_DELTA_RESOLVER_FILTERED].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_MotorPositionParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_MotorPositionParameters].data, sizeof(data));
        canData[INVERTER_RR_MotorPositionParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_MOTOR_ANGLE].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_MOTOR_SPEED].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_ELECTRICAL_OUTPUT_FREQUENCY].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_DELTA_RESOLVER_FILTERED].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Current Parameters CAN message. (6)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_CurrentParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_CurrentParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_CurrentParameters].data, sizeof(data));
        canData[INVERTER_RL_CurrentParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_PHASE_A_CURRENT].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_PHASE_B_CURRENT].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_PHASE_C_CURRENT].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_DC_BUS_CURRENT].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_CurrentParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_CurrentParameters].data, sizeof(data));
        canData[INVERTER_RR_CurrentParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_PHASE_A_CURRENT].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_PHASE_B_CURRENT].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_PHASE_C_CURRENT].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_DC_BUS_CURRENT].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Voltage Parameters CAN message. (7)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_VoltageParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_VoltageParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_VoltageParameters].data, sizeof(data));
        canData[INVERTER_RL_VoltageParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_DC_BUS_VOLTAGE].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_OUTPUT_VOLTAGE].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_VAB_VD_VOLTAGE].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_VAB_VQ_VOLTAGE].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_VoltageParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_VoltageParameters].data, sizeof(data));
        canData[INVERTER_RR_VoltageParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_DC_BUS_VOLTAGE].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_OUTPUT_VOLTAGE].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_VAB_VD_VOLTAGE].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_VAB_VQ_VOLTAGE].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Flux Parameters CAN message. (8)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_FluxParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_FluxParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_FluxParameters].data, sizeof(data));
        canData[INVERTER_RL_FluxParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_FLUX_COMMAND].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_FLUX_FEEDBACK].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_ID_CURRENT].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_IQ_CURRENT].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_FluxParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_FluxParameters].data, sizeof(data));
        canData[INVERTER_RR_FluxParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_FLUX_COMMAND].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_FLUX_FEEDBACK].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_ID_CURRENT].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_IQ_CURRENT].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Internal Voltage Paramaters CAN message. (9)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_InternalVoltageParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_InternalVoltageParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_InternalVoltageParameters].data, sizeof(data));
        canData[INVERTER_RL_InternalVoltageParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_1_5_REFERENCE_VOLTAGE].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_2_5_REFERENCE_VOLTAGE].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_5_0_REFERENCE_VOLTAGE].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_12_0_REFERENCE_VOLTAGE].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_InternalVoltageParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_InternalVoltageParameters].data, sizeof(data));
        canData[INVERTER_RR_InternalVoltageParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_1_5_REFERENCE_VOLTAGE].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_2_5_REFERENCE_VOLTAGE].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_5_0_REFERENCE_VOLTAGE].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_12_0_REFERENCE_VOLTAGE].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Internal States CAN message. (10)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_InternalStateParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_InternalStateParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_InternalStateParameters].data, sizeof(data));
        canData[INVERTER_RL_InternalStateParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_VSM_STATE].data = data[1];
        InverterRLData[INVERTER_PWM_FREQUENCY].data = data[0];
        InverterRLData[INVERTER_STATE].data = data[3];
        InverterRLData[INVERTER_RELAY_1_STATUS].data = (data[2] >> 0) & 0x01;
        InverterRLData[INVERTER_RELAY_2_STATUS].data = (data[2] >> 1) & 0x01;
        InverterRLData[INVERTER_RELAY_3_STATUS].data = (data[2] >> 2) & 0x01;
        InverterRLData[INVERTER_RELAY_4_STATUS].data = (data[2] >> 3) & 0x01;
        InverterRLData[INVERTER_RELAY_5_STATUS].data = (data[2] >> 4) & 0x01;
        InverterRLData[INVERTER_RELAY_6_STATUS].data = (data[2] >> 5) & 0x01;
        InverterRLData[INVERTER_RUN_MODE].data = (data[5] >> 0) & 0x01;
        InverterRLData[INVERTER_SELF_SENSING_ASSIST].data = (data[5] >> 1) & 0x01;
        InverterRLData[INVERTER_ACTIVE_DISCHARGE_STATE].data = (data[5] >> 5) & 0x07;
        InverterRLData[INVERTER_COMMAND_MODE].data = (data[4] >> 0) & 0x01;
        InverterRLData[INVERTER_ROLLING_COUNTER_VALUE].data = (data[4] >> 4) & 0x0F;
        InverterRLData[INVERTER_ENABLE_STATE].data = (data[7] >> 0) & 0x01;
        InverterRLData[INVERTER_BURST_MODEL_MODE].data = (data[7] >> 0) & 0x01;
        InverterRLData[INVERTER_START_MODE_ACTIVE].data = (data[7] >> 6) & 0x01;
        InverterRLData[INVERTER_ENABLE_LOCKOUT].data = (data[7] >> 7) & 0x01;
        InverterRLData[INVERTER_DIRECTION_COMMAND].data = (data[6] >> 0) & 0x01;
        InverterRLData[INVERTER_BMS_ACTIVE].data = (data[6] >> 1) & 0x01;
        InverterRLData[INVERTER_BMS_LIMITING_TORQUE].data = (data[6] >> 2) & 0x01;
        InverterRLData[INVERTER_LIMIT_MAX_SPEED].data = (data[6] >> 3) & 0x01;
        InverterRLData[INVERTER_LIMIT_HOT_SPOT].data = (data[6] >> 4) & 0x01;
        InverterRLData[INVERTER_LOW_SPEED_LIMITING].data = (data[6] >> 5) & 0x01;
        InverterRLData[INVERTER_COOLANT_TEMP_LIMITING].data = (data[6] >> 6) & 0x01;
        InverterRLData[INVERTER_LIMIT_STALL_BURST_MODEL].data = (data[6] >> 7) & 0x01;
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_InternalStateParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_InternalStateParameters].data, sizeof(data));
        canData[INVERTER_RR_InternalStateParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_VSM_STATE].data = data[1];
        InverterRRData[INVERTER_PWM_FREQUENCY].data = data[0];
        InverterRRData[INVERTER_STATE].data = data[3];
        InverterRRData[INVERTER_RELAY_1_STATUS].data = (data[2] >> 0) & 0x01;
        InverterRRData[INVERTER_RELAY_2_STATUS].data = (data[2] >> 1) & 0x01;
        InverterRRData[INVERTER_RELAY_3_STATUS].data = (data[2] >> 2) & 0x01;
        InverterRRData[INVERTER_RELAY_4_STATUS].data = (data[2] >> 3) & 0x01;
        InverterRRData[INVERTER_RELAY_5_STATUS].data = (data[2] >> 4) & 0x01;
        InverterRRData[INVERTER_RELAY_6_STATUS].data = (data[2] >> 5) & 0x01;
        InverterRRData[INVERTER_RUN_MODE].data = (data[5] >> 0) & 0x01;
        InverterRRData[INVERTER_SELF_SENSING_ASSIST].data = (data[5] >> 1) & 0x01;
        InverterRRData[INVERTER_ACTIVE_DISCHARGE_STATE].data = (data[5] >> 5) & 0x07;
        InverterRRData[INVERTER_COMMAND_MODE].data = (data[4] >> 0) & 0x01;
        InverterRRData[INVERTER_ROLLING_COUNTER_VALUE].data = (data[4] >> 4) & 0x0F;
        InverterRRData[INVERTER_ENABLE_STATE].data = (data[7] >> 0) & 0x01;
        InverterRRData[INVERTER_BURST_MODEL_MODE].data = (data[7] >> 0) & 0x01;
        InverterRRData[INVERTER_START_MODE_ACTIVE].data = (data[7] >> 6) & 0x01;
        InverterRRData[INVERTER_ENABLE_LOCKOUT].data = (data[7] >> 7) & 0x01;
        InverterRRData[INVERTER_DIRECTION_COMMAND].data = (data[6] >> 0) & 0x01;
        InverterRRData[INVERTER_BMS_ACTIVE].data = (data[6] >> 1) & 0x01;
        InverterRRData[INVERTER_BMS_LIMITING_TORQUE].data = (data[6] >> 2) & 0x01;
        InverterRRData[INVERTER_LIMIT_MAX_SPEED].data = (data[6] >> 3) & 0x01;
        InverterRRData[INVERTER_LIMIT_HOT_SPOT].data = (data[6] >> 4) & 0x01;
        InverterRRData[INVERTER_LOW_SPEED_LIMITING].data = (data[6] >> 5) & 0x01;
        InverterRRData[INVERTER_COOLANT_TEMP_LIMITING].data = (data[6] >> 6) & 0x01;
        InverterRRData[INVERTER_LIMIT_STALL_BURST_MODEL].data = (data[6] >> 7) & 0x01;
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit Fault Codes CAN message. (11)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_FaultCodes(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_FaultCodes].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_FaultCodes].data, sizeof(data));
        canData[INVERTER_RL_FaultCodes].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_POST_FAULT_LO].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_POST_FAULT_HI].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_RUN_FAULT_LO].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_RUN_FAULT_HI].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_FaultCodes].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_FaultCodes].data, sizeof(data));
        canData[INVERTER_RR_FaultCodes].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_POST_FAULT_LO].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_POST_FAULT_HI].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_RUN_FAULT_LO].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_RUN_FAULT_HI].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}

/**
 * @brief Parses Inverter 29-bit High Speed CAN message. (0x0B0)
 * @param uint8_t data[8]: Array of 8 bytes of data to be parsed
 * @retval None
 */
void CAN_Parse_Inverter_HighSpeedParameters(Inverter inverter) {
    xSemaphoreTake(canMutex, portMAX_DELAY);
    uint8_t data[8] = {0};
    if (inverter == RL) {
        if (canData[INVERTER_RL_HighSpeedParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RL_HighSpeedParameters].data, sizeof(data));
        canData[INVERTER_RL_HighSpeedParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRLDataMutex, portMAX_DELAY);
        InverterRLData[INVERTER_TORQUE_COMMAND_HS].data = (data[1] << 8) | data[0];
        InverterRLData[INVERTER_TORQUE_FEEDBACK_HS].data = (data[3] << 8) | data[2];
        InverterRLData[INVERTER_MOTOR_SPEED_HS].data = (data[5] << 8) | data[4];
        InverterRLData[INVERTER_DC_BUS_VOLTAGE_HS].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRLDataMutex);
    } else if (inverter == RR) {
        if (canData[INVERTER_RR_HighSpeedParameters].parsed) {
            xSemaphoreGive(canMutex);
            return;
        }
        memcpy(data, canData[INVERTER_RR_HighSpeedParameters].data, sizeof(data));
        canData[INVERTER_RR_HighSpeedParameters].parsed = true;
        xSemaphoreGive(canMutex);
        xSemaphoreTake(InverterRRDataMutex, portMAX_DELAY);
        InverterRRData[INVERTER_TORQUE_COMMAND_HS].data = (data[1] << 8) | data[0];
        InverterRRData[INVERTER_TORQUE_FEEDBACK_HS].data = (data[3] << 8) | data[2];
        InverterRRData[INVERTER_MOTOR_SPEED_HS].data = (data[5] << 8) | data[4];
        InverterRRData[INVERTER_DC_BUS_VOLTAGE_HS].data = (data[7] << 8) | data[6];
        xSemaphoreGive(InverterRRDataMutex);
    }
}