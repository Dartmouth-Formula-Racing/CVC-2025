/*
 * torque.h
 *
 * Created on 6/7/2025
 * Andrei Gerashchenko
 *
 */

#ifndef CVC_TORQUE_H
#define CVC_TORQUE_H

#define TORQUE_MAX 100.0f           // Peak torque at motor (Nm), EMRAX 188 datasheet (S2 30 s)
#define REVERSE_TORQUE_LIMIT 0.30f  // Percentage of nominal torque (0.0 - 100.0)
#define DISABLE_ON_ZERO_THROTTLE 0  // Disable inverters when throttle is at 0%

#define LEFT_MOTOR_ENABLE 1   // Left motor enable bit
#define RIGHT_MOTOR_ENABLE 1  // Right motor enable bit

#define LEFT_MOTOR_DIRECTION_INVERTED 0U
#define RIGHT_MOTOR_DIRECTION_INVERTED 1U

#define TORQUE_VECTORING_GAIN 0.00  // Gain for torque vectoring
#define TORQUE_VECTORING_DEADBAND 0.06f  // No vectoring effect below this normalized steering magnitude
#define TORQUE_VECTORING_CURVE 2.0f      // Exponential ramp after deadband (2.0 = quadratic)
// Cursed Wrapping Limits
#define STEERING_LEFT_LIMIT   3559U
#define STEERING_RIGHT_LIMIT  6286U   // 2190 + 4096
#define STEERING_WRAP_POINT   3000U   // threshold between 2579 and 3559
#define STEERING_DEAD_GAP_LOW 2250U
#define STEERING_DEAD_GAP_HIGH 3500U

// ---------------------------------------------------------------------------
// Motor - EMRAX 188 MV, datasheet v1.7
// ---------------------------------------------------------------------------
#define MOTOR_PEAK_POWER_KW 60.0f        // S2 30 s, at 6500 rpm
#define MOTOR_MAX_SPEED_RPM 8000.0f      // Limiting speed
#define MOTOR_KT_NM_PER_ARMS 0.32f       // Torque constant, MV winding
#define MOTOR_PEAK_CURRENT_ARMS 310.0f   // Current needed for MOTOR peak torque (100 Nm)
#define TORQUE_CONSTANT MOTOR_KT_NM_PER_ARMS  // Nm/Arms (legacy name)
#define RPM_TO_RADS 0.1047197551            // 2 * pi / 60

// ---------------------------------------------------------------------------
// Inverter - Cascadia PM100DZ, PM family datasheet
// ---------------------------------------------------------------------------
#define INVERTER_PEAK_CURRENT_ARMS 200.0f  // "Peak" is defined as 30 s maximum
#define INVERTER_CONT_CURRENT_ARMS 150.0f
#define INVERTER_PEAK_DC_POWER_KW 100.0f   // per inverter
#define INVERTER_MIN_DC_VOLTAGE 300.0f     // Lower end of the PM100DZ operating range

// Iq limit in peak amps = 200 Arms * sqrt(2) = 282.8 A  ->  EEPROM IQ_Limit_EEPROM_(Amps)_x_10 = 2828
#define INVERTER_IQ_LIMIT_A (INVERTER_PEAK_CURRENT_ARMS * 1.41421356f)

// Torque the motor can really produce at the inverter current limit (EMRAX: I[Arms] = T / Kt):
//   200 Arms * 0.32 Nm/Arms = 64 Nm per motor.  The motor itself could do 100 Nm (needs 310 Arms),
//   so the PM100DZ, not the EMRAX, is the torque limit of this powertrain.
#define INVERTER_LIMITED_MOTOR_TORQUE_NM (INVERTER_PEAK_CURRENT_ARMS * MOTOR_KT_NM_PER_ARMS)

// The inverter works in units of its *own* torque estimate: T_est = 1.5 * pole_pairs * Veh_Flux * Iq
//   = 1.5 * 10 * 0.019 Wb = 0.285 Nm per peak amp.  At the Iq limit that is 80.6 Nm.
#define INVERTER_TORQUE_PER_AMP 0.285f
#define INVERTER_TORQUE_CEILING_NM (INVERTER_IQ_LIMIT_A * INVERTER_TORQUE_PER_AMP)

// Inverter EEPROM Motor_Torque_Limit_EEPROM_(Nm)_x_10 = 800. Must stay <= the current-limit ceiling above.
#define INVERTER_TORQUE_LIMIT_NM 80.0f

// Motor-shaft Nm -> inverter-estimate Nm (team calibration: command 121 to get 100 at the motor)
#define TORQUE_COMMAND_SCALE (121.0 / 100.0)

// 100% pedal = the most torque the inverters will actually deliver. Commanding more than this only
// created dead pedal travel (the old 121 Nm full scale saturated at ~61% pedal).
#define TORQUE_CMD_FULL_SCALE_NM                                                   \
    (((TORQUE_MAX * TORQUE_COMMAND_SCALE) < INVERTER_TORQUE_LIMIT_NM)              \
         ? (float)(TORQUE_MAX * TORQUE_COMMAND_SCALE)                              \
         : INVERTER_TORQUE_LIMIT_NM)

// Compile-time guard: the pedal full scale must be reachable at the configured Iq limit.
_Static_assert((int)(INVERTER_TORQUE_LIMIT_NM * 10.0f) <= (int)(INVERTER_TORQUE_CEILING_NM * 10.0f),
               "Motor_Torque_Limit exceeds what the Iq limit can deliver - raise Iq or lower the torque limit");

// ---------------------------------------------------------------------------
// Power limit (0x202 message to both inverters)
// ---------------------------------------------------------------------------
#define MAX_POWER 80.0f  // Max total DC bus power (kW) at the accumulator, both inverters combined
// Tolerance on the rules limit: the inverter sees the bus after the harness and its DC current is an estimate,
// the energy meter measures at the accumulator. Set to 1.0f to use the full MAX_POWER.
#define POWER_LIMIT_MARGIN 0.98f

#define MAX_BSPD_POWER 4.5f  // Max power under hard braking (kW), designed to not trip BSPD

// Per-inverter DC current ceiling: PM100DZ 100 kW peak / 300 V minimum operating voltage = 333 A
#define INVERTER_CURRENT_LIMIT (INVERTER_PEAK_DC_POWER_KW * 1000.0f / INVERTER_MIN_DC_VOLTAGE)

#define MAX_REGEN_CURRENT 50  // Max regenerative braking current (A)

// Bus voltage used to turn the power limit into a current limit.
// Primary: voltage measured by the inverters themselves (message 0x..7, ~30 ms) - what they regulate against.
// Fallback: EMUS pack total (1 Hz).
#define INVERTER_VDC_STALE_TIMEOUT_MS 150U  // Reject inverter bus voltage older than this
#define BUS_VOLTAGE_MIN_VALID_V 50.0f       // Below this the bus is not energised yet
#define BMS_VOLTAGE_STALE_TIMEOUT_MS 1000U  // Reject pack voltage if BMS has been silent for >1 s
#define BMS_VOLTAGE_LIMIT_HOLD_TIMEOUT_MS (3U * BMS_VOLTAGE_STALE_TIMEOUT_MS)  // Hold last valid limits for ~3 BMS periods before fail-safe
#define TORQUE_LIMIT_FAILSAFE_DISCHARGE_A 50.0f  // Conservative non-zero discharge limit while bus voltage is unavailable
#define TORQUE_LIMIT_FAILSAFE_CHARGE_A 25.0f    // Conservative non-zero charge limit while bus voltage is unavailable

// The EMUS knows the real cell limits (configured in the BMS); when it raises "power reduction" or a
// low-cell-voltage / high-discharge-current warning, scale the power limit instead of running into a BMS trip.
#define BMS_DERATE_ENABLE 1
#define BMS_POWER_REDUCTION_SCALE 0.5f

typedef struct {
    float rearLeft;
    float rearRight;
    float frontLeft;
    float frontRight;
} TorqueValues;

void Torque_Init(void);
void Torque_SendInverterFaultClear(void);

TorqueValues Torque_GetValues(void);

#endif  // CVC_TORQUE_H
