#ifndef __CONFIG_H
#define __CONFIG_H

#include "m5stack-bus.h"

#define VERSIONSTR "20230412"

#define MOTOR_ANGLE_DEG         28.5
#define BASE_Z_ROTATION_DEG     30

// Carefully tuned magic rotation parameters for 149mm glass globe
#define STEPS_PER_ROTATION_X         (2*4050)
#define STEPS_PER_ROTATION_Y         (2*3945)
#define STEPS_PER_ROTATION_Z         (2*4275)
#define STEPS_PER_ROTATION_H         (2*4130)

#define MOTORS_MAX_SPEED        20000
#define MOTORS_MAX_ACCEL		5000

// PINS

#endif // __CONFIG_H
