/*
 * analogs.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef ANALOGS_H
#define ANALOGS_H

#include <semphr.h>

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

extern SemaphoreHandle_t analogMutex;

#define ANALOG_INVALID 0xFFFF

void Analogs_Init(void);
uint16_t Analogs_ReadChannel(AnalogChannel ch);

#endif  // ANALOGS_H