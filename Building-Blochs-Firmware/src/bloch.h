#ifndef __BLOCH_H
#define __BLOCH_H

#include <Arduino.h>
#include <ArduinoEigenDense.h>

Eigen::Matrix3d generateMotorMatrix(double wheel_angle_degrees, double z_rotation_degrees);
void printMatrix3d(Eigen::Matrix3d& m, const char* name = "M");

#endif // __BLOCH_H