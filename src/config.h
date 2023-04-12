#ifndef __CONFIG_H
#define __CONFIG_H

#define VERSIONSTR "20230412"

#define MOTOR_ANGLE_DEG         28.5
#define BASE_Z_ROTATION_DEG     30

// Carefully tuned magic rotation parameters for 149mm glass globe
#define HALF_ROTATION_X         4050
#define HALF_ROTATION_Y         3945
#define HALF_ROTATION_Z         4275
#define HALF_ROTATION_H         4130
#define MAX_SPEED               20000.0
#define MAX_ACCELERATION        5000.0
#define VECTOR_X                {1, 0, 0}
#define VECTOR_Y                {0, 1, 0}
#define VECTOR_Z                {0, 0, 1}
#define VECTOR_H                {0.707106, 0, 0.707106} // sqrt(2)/2

// LED Colors
#define HUE_RED                 0
#define HUE_GREEN               0.33
#define HUE_BLUE                0.667
#define HUE_YELLOW              0.083
#define LED_COUNT               8
#define LED_PIN                 16
#define LED_COLOR_SATURATION    128

// PINS
#define STPA_DIR 26
#define STPA_STEP 25

#define STPB_DIR 21
#define STPB_STEP 17

#define STPC_DIR 33
#define STPC_STEP 15

#define STP_EN 18
#define STP_MS1 23
#define STP_MS2 22
#define STP_MS3 32

#define BUT_RED 34
#define BUT_GREEN 39
#define BUT_BLUE 36
#define BUT_YELLOW 4


#endif // __CONFIG_H
