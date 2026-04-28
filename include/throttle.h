/*
 * throttle.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef CVC_THROTTLE_H
#define CVC_THROTTLE_H

#define APPS1_MIN 2215
#define APPS1_MAX 3900

#define APPS2_MIN 150
#define APPS2_MAX 1825

#define THROTTLE_TOLERANCE 0.10f
#define THROTTLE_MIN_VALID_TIME 750

void Throttle_Init(void);

bool Throttle_Valid(void);
float Throttle_GetValue(void);

#endif  // CVC_THROTTLE_H