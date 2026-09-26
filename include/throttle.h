/*
 * throttle.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#ifndef CVC_THROTTLE_H
#define CVC_THROTTLE_H

#define APPS1_MIN 2055
#define APPS1_MAX 3400

#define APPS2_MIN 10
#define APPS2_MAX 1485

#define THROTTLE_TOLERANCE 0.30f
#define THROTTLE_MIN_VALID_TIME 750

void Throttle_Init(void);

bool Throttle_Valid(void);
float Throttle_GetValue(void);

#endif  // CVC_THROTTLE_H