/*
 * wheelspeeds.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef WHEEL_SPEEDS_H
#define WHEEL_SPEEDS_H

#define COUNT_PER_REVOLUTION 40 // Tone ring teeth count

void WheelSpeeds_Init(void);

float wheelSpeed_FL(void);
float wheelSpeed_FR(void);
float wheelSpeed_RL(void);
float wheelSpeed_RR(void);

#endif  // WHEEL_SPEEDS_H