/*
 * analogs.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef ANALOGS_H
#define ANALOGS_H

typedef enum {
    APPS_1,
    APPS_2,
    Current_Sensor,
    Steering_Angle,
    Inverter1_CS,
    Inverter2_CS,
    Pump1_CS,
    Pump2_CS,
    Battery_Voltage,
    DCDC_Voltage,
    Brake_Pressure,
    GLV_Bus_Voltage,
    ADC_CHANNEL_COUNT  // Total number of analog channels, must be last
} AnalogChannel;

#define ANALOG_INVALID 0xFFFF

#define ADC_POLL_TIMEOUT_MS 5

void Analogs_Init(void);
uint16_t Analogs_ReadChannel(AnalogChannel ch);
uint32_t Analogs_GetTimeoutCount(void);

#endif  // ANALOGS_H