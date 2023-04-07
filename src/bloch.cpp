#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoEigenDense.h>

#include "bloch.h"

using namespace Eigen;

// Previous matrices:
// Generated with angle = 35 degress, no z_rot
// Matrix3d MotorMapping = {0.81915204,  -0.,         0.57357644,            //cos(35),0,sin(35)
//                          -0.40957602, -0.70940648, 0.57357644,            //-cos(35)/2,-(sqrt(3)/2)*cos(35),sin(35)
//                          -0.40957602,  0.70940648, 0.57357644};             //-cos(35)/2,(sqrt(3)/2)*cos(35),sin(35)
// Generated with angle = 28.5 degrees, no z_rot
// Matrix3d MotorMapping = {0.87881711, 0.00000000, 0.47715876,
//                          -0.43940856, -0.76107794, 0.47715876,
//                          -0.43940856, 0.76107794, 0.47715876};
// Generated with block.js with angle = 28.5 and z_rot = PI/6
// MotorMapping << 0.76107794, -0.43940856, 0.47715876,
//             -0.76107794, -0.43940856, 0.47715876,
//             -0.00000000, 0.87881711, 0.47715876;

Matrix3d generateMotorMatrix(double /*mount_angle*/ wa_deg, double z_rot_deg)
{
    // function compute() {
    //     var wheel_angle_degrees = parseFloat(document.getElementById('angle').value)
    //     var wheel_angle = wheel_angle_degrees / 180 * Math.PI
    double wheel_angle = wa_deg * PI / 180;
    double z_rot = z_rot_deg * PI / 180;
    // var wheel_vectors = [0., 2.*Math.PI/3, 4.*Math.PI/3].map((phi) => [Math.sin(wheel_angle) * Math.cos(phi), Math.sin(wheel_angle) * Math.sin(phi), -Math.cos(wheel_angle)])
    // double wheel_vectors = 
    //   map [0, 2/3*PI, 4/3*PI] onto [sin(wheel_angle)*cos(phi), 
    //                                 sin(wheel_angle)*sin(phi), 
    //                                 -cos(wheel_angle)]
    double phi;
    Vector3d wheel_vector[3]; // Wheel direction seen from center of ball
    phi = 0 + z_rot;
    wheel_vector[0] << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);
    phi = 2*PI/3 + z_rot;
    wheel_vector[1] << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);
    phi = 4*PI/3 + z_rot;
    wheel_vector[2] << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);

    // var wheel_normal_vectors = 
    // [0., 2.*Math.PI/3, 4.*Math.PI/3].map((phi) => 
    //      [Math.cos(wheel_angle) * Math.cos(phi), 
    //       Math.cos(wheel_angle) * Math.sin(phi), 
    //       Math.sin(wheel_angle)])
    //          .map((x) => [ .. ] // Normalize
    Vector3d wheel_normal[3];
    phi = 0 + z_rot;
    wheel_normal[0] << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    phi = 2*PI/3 + z_rot;
    wheel_normal[2] << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    phi = 4*PI/3 + z_rot;
    wheel_normal[3] << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    // wheel_normal[0].normalize();
    // wheel_normal[1].normalize();
    // wheel_normal[2].normalize();

    // var wheel_direction_vectors = 
    //  wheel_vectors.map((a,i) => 
    // [a[1] * wheel_normal_vectors[i][2] - a[2] * wheel_normal_vectors[i][1],   -> a_y*n_z-a_z*n_y
    //  a[2] * wheel_normal_vectors[i][0] - a[0] * wheel_normal_vectors[i][2], 
    //  a[0] * wheel_normal_vectors[i][1] - a[1] * wheel_normal_vectors[i][0]])
    //      .map((x) => [ .. ] // Normalize
    Vector3d wheel_dir[3];
    wheel_dir[0] = wheel_vector[0].cross(wheel_normal[0]).normalized();
    wheel_dir[1] = wheel_vector[1].cross(wheel_normal[1]).normalized();
    wheel_dir[2] = wheel_vector[2].cross(wheel_normal[2]).normalized();

//     var coeff_matrix = wheel_vectors.map((a,i) => 
            // [a[1] * wheel_direction_vectors[i][2] - a[2] * wheel_direction_vectors[i][1], 
            //  a[2] * wheel_direction_vectors[i][0] - a[0] * wheel_direction_vectors[i][2], 
            //  a[0] * wheel_direction_vectors[i][1] - a[1] * wheel_direction_vectors[i][0]])
    Vector3d wheel_motion[3];
    wheel_motion[0] = wheel_vector[0].cross(wheel_dir[0]);
    wheel_motion[1] = wheel_vector[1].cross(wheel_dir[1]);
    wheel_motion[2] = wheel_vector[2].cross(wheel_dir[2]);
//     coeff_matrix = coeff_matrix.map((x) => [-x[0], x[1], -x[2]])
    Matrix3d m;
    m <<    -wheel_motion[0](0), wheel_motion[0](1), -wheel_motion[0](2),
            -wheel_motion[1](0), wheel_motion[1](1), -wheel_motion[1](2),
            -wheel_motion[2](0), wheel_motion[2](1), -wheel_motion[2](2);

    Serial.println("M = ");
    Serial.printf("\t%0.2f %0.2f %0.2f\n", m(0,0), m(0,1), m(0,2));
    Serial.printf("\t%0.2f %0.2f %0.2f\n", m(1,0), m(1,1), m(1,2));
    Serial.printf("\t%0.2f %0.2f %0.2f\n", m(2,0), m(2,1), m(2,2));

    // Matrix3d mc = getMotorMatrix();
    // Serial.println("M_check = ");
    // Serial.printf("\t%0.2f %0.2f %0.2f\n", mc(0,0), mc(0,1), m(0,2));
    // Serial.printf("\t%0.2f %0.2f %0.2f\n", mc(1,0), mc(1,1), m(1,2));
    // Serial.printf("\t%0.2f %0.2f %0.2f\n", mc(2,0), mc(2,1), m(2,2));

    return m;
};
