/*
 * throttle.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef CVC_THROTTLE_H
#define CVC_THROTTLE_H

#define APPS1_MIN 2245
#define APPS1_MAX 3320

#define APPS2_MIN 180
#define APPS2_MAX 1350

#define THROTTLE_TOLERANCE 0.10f
#define THROTTLE_MIN_VALID_TIME 750

void Throttle_Init(void);

bool Throttle_Valid(void);
float Throttle_GetValue(void);

#endif  // CVC_THROTTLE_H