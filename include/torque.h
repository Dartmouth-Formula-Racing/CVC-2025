/*
 * torque.c
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 */

#define TORQUE_MAX 100.0f           // Max torque at motor (Nm)
#define REVERSE_TORQUE_LIMIT 0.30f  // Percentage of nominal torque (0.0 - 100.0)
#define DISABLE_ON_ZERO_THROTTLE 0  // Disable inverters when throttle is at 0%

#define LEFT_MOTOR_ENABLE 1   // Left motor enable bit
#define RIGHT_MOTOR_ENABLE 1  // Right motor enable bit

#define LEFT_MOTOR_DIRECTION_INVERTED 0U
#define RIGHT_MOTOR_DIRECTION_INVERTED 1U

#define TORQUE_VECTORING_GAIN 0.70  // Gain for torque vectoring
#define TORQUE_VECTORING_DEADBAND 0.06f  // No vectoring effect below this normalized steering magnitude
#define TORQUE_VECTORING_CURVE 2.0f      // Exponential ramp after deadband (2.0 = quadratic)
// Cursed Wrapping Limits
#define STEERING_LEFT_LIMIT   3559U
#define STEERING_RIGHT_LIMIT  6286U   // 2190 + 4096
#define STEERING_WRAP_POINT   3000U   // threshold between 2579 and 3559
#define STEERING_DEAD_GAP_LOW 2250U
#define STEERING_DEAD_GAP_HIGH 3500U


#define TORQUE_CONSTANT 0.32                // Nm/Arms
#define RPM_TO_RADS 0.1047197551            // 2 * pi / 60
#define INVERTER_CURRENT_LIMIT 270.0        // Amps
#define BUS_MIN_VOLTAGE 100.0               // Volts
#define BUS_RESISTANCE 0.275                // Ohms
#define TORQUE_COMMAND_SCALE (121.0 / 100.0)  // Need to command 121 Nm to inverter to get 100 Nm at motor

#define MAX_POWER 80.0f  // Max total DC bus power (kW)

#define MAX_BSPD_POWER 4.5f  // Max power under hard braking (kW), designed to not trip BSPD

#define MAX_REGEN_CURRENT 50  // Max regenerative braking current (A)
#define BMS_VOLTAGE_STALE_TIMEOUT_MS 1000U  // Reject pack voltage if BMS has been silent for >1 s

typedef struct {
    float rearLeft;
    float rearRight;
    float frontLeft;
    float frontRight;
} TorqueValues;

void Torque_Init(void);
void Torque_SendInverterFaultClear(void);

TorqueValues Torque_GetValues(void);