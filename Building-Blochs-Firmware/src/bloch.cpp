#include "bloch.h"

#include <stdlib.h>

#include "config.h"
#include "log.h"
#include "m5stack-data.h"

// local functions
using namespace Eigen;
Eigen::Matrix3d generateMotorMatrix(double wheel_angle_degrees, double z_rotation_degrees);
void            printMatrix3d(Eigen::Matrix3d& m, const char* name = "M");


BlochSphere::BlochSphere() : 
    _motor1(AccelStepper::DRIVER, M5_STEPMOTORDRIVER_STP_X, M5_STEPMOTORDRIVER_DIR_X),
    _motor2(AccelStepper::DRIVER, M5_STEPMOTORDRIVER_STP_Y, M5_STEPMOTORDRIVER_DIR_Y),
    _motor3(AccelStepper::DRIVER, M5_STEPMOTORDRIVER_STP_Z, M5_STEPMOTORDRIVER_DIR_Z)
{
};

BlochSphere::~BlochSphere()
{

};

bool BlochSphere::begin()
{
    if(!_driver.init())
    {
        WARNING("_driver returned false. error?");
    };

    // Reset Fault status
    // _driver.setMicrostepResolution(Module_Stepmotor::kMicrosteps8);
    _motor1.enableOutputs();
    _motor2.enableOutputs();
    _motor3.enableOutputs();

    uint8_t status;
    _driver.getFaultStatus(&status); 
    DBG("FaultStatus: 0x%0x", status); // probably 7, ok?

    // Generate Movement Matrix
    _motormatrix = generateMotorMatrix(MOTOR_ANGLE_DEG, BASE_Z_ROTATION_DEG);
    printMatrix3d(_motormatrix, "MotorMatrix");

    _state = RESET;

    return true;
};

bool BlochSphere::rotate(const Vector3d axis, const int steps)
{
    DBG("Queue move");
    _queue.push(_motormatrix * axis.normalized() * steps);
    return true;
};

void BlochSphere::loop()
{
    Vector3d move;
    float max_move;

    static state_t _prvstate = RESET;
    if(_state != _prvstate)
    {
        DBG("%lu: State %s -> %s", millis(), StateNames[_prvstate], StateNames[_state]);
        _prvstate = _state;
    };

    switch(_state)
    {
        case RESET:
            _driver.enableMotor(0);
            _driver.resetMotor(0, 0);    
            _driver.resetMotor(1, 0);    
            _driver.resetMotor(2, 0);

            _state = IDLE;
            break;

        case IDLE:
            if(!_queue.empty())
            {
                _state = MOVE_START;
                break;
            };
            return;

        case MOVE_START:
            // Test for next move or go IDLE
            if(_queue.empty())
            {
                _state = IDLE;
                break;
            };

            // pop move from queue
            move = _queue.front();
            _queue.pop();

            // Start executing move
            _driver.enableMotor(1);
            // delay(30);

            //FIXME: instead of max() use the norm of the vector here and for speed
            //  or maybe the inner product of the update for the axis
            max_move = max(max(abs(move[0]), abs(move[1])), abs(move[2]));

            _motor1.setAcceleration(move[2] / max_move * MOTORS_MAX_ACCEL);
            _motor2.setAcceleration(move[1] / max_move * MOTORS_MAX_ACCEL);
            _motor3.setAcceleration(move[0] / max_move * MOTORS_MAX_ACCEL);
            // DBG("ACCEL: %f %f %f",
            //     move[2] / max_move * MOTORS_MAX_ACCEL,
            //     move[1] / max_move * MOTORS_MAX_ACCEL,
            //     move[0] / max_move * MOTORS_MAX_ACCEL
            // );

            _motor1.setMaxSpeed(move[2] / max_move * MOTORS_MAX_SPEED);
            _motor2.setMaxSpeed(move[1] / max_move * MOTORS_MAX_SPEED);
            _motor3.setMaxSpeed(move[0] / max_move * MOTORS_MAX_SPEED);
            // DBG("SPEED: %f %f %f",
            //     move[2] / max_move * MOTORS_MAX_SPEED,
            //     move[1] / max_move * MOTORS_MAX_SPEED,
            //     move[0] / max_move * MOTORS_MAX_SPEED
            // );

            DBG("%lu: Move (%.0f, %.0f, %.0f) (max=%.0f)", millis(), move[0], move[1], move[2], max_move);
            _motor1.move(move[2]);
            _motor2.move(move[1]);
            _motor3.move(move[0]);

            _state = MOVE_BUSY;
            break;

        case MOVE_BUSY:
            // Wait until all motors have finished
            _motor1.run();
            _motor2.run();
            _motor3.run();

            if(_motor1.distanceToGo())
                break;
            if(_motor2.distanceToGo())
                break;
            if(_motor3.distanceToGo())
                break;
            
            _state = MOVE_END;
            break;
        
        case MOVE_END:
            _driver.enableMotor(0);

            // Start next or go idle
            if(_queue.empty())
                _state = IDLE;
            else
                _state = MOVE_START;
            break;
    }; // while switch
};

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
    // Vector3d wheel_vector[3]; // Wheel direction seen from center of ball
    Matrix3d wheel_vector;
    phi = 0 + z_rot;
    wheel_vector.row(0) << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);
    phi = 2*PI/3 + z_rot;
    wheel_vector.row(1) << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);
    phi = 4*PI/3 + z_rot;
    wheel_vector.row(2) << sin(wheel_angle)*cos(phi), sin(wheel_angle)*sin(phi), -cos(wheel_angle);

    // var wheel_normal_vectors = 
    // [0., 2.*Math.PI/3, 4.*Math.PI/3].map((phi) => 
    //      [Math.cos(wheel_angle) * Math.cos(phi), 
    //       Math.cos(wheel_angle) * Math.sin(phi), 
    //       Math.sin(wheel_angle)])
    //          .map((x) => [ .. ] // Normalize
    Matrix3d wheel_normal;
    phi = 0 + z_rot;
    wheel_normal.row(0) << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    phi = 2*PI/3 + z_rot;
    wheel_normal.row(1) << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    phi = 4*PI/3 + z_rot;
    wheel_normal.row(2) << cos(wheel_angle)*cos(phi), cos(wheel_angle)*sin(phi), sin(wheel_angle);
    // wheel_normal[0].normalize();
    // wheel_normal[1].normalize();
    // wheel_normal[2].normalize();

    // var wheel_direction_vectors = 
    //  wheel_vectors.map((a,i) => 
    // [a[1] * wheel_normal_vectors[i][2] - a[2] * wheel_normal_vectors[i][1],   -> a_y*n_z-a_z*n_y
    //  a[2] * wheel_normal_vectors[i][0] - a[0] * wheel_normal_vectors[i][2], 
    //  a[0] * wheel_normal_vectors[i][1] - a[1] * wheel_normal_vectors[i][0]])
    //      .map((x) => [ .. ] // Normalize
    Matrix3d wheel_dir;
    wheel_dir.row(0) << wheel_vector.row(0).cross(wheel_normal.row(0));
    wheel_dir.row(1) << wheel_vector.row(1).cross(wheel_normal.row(1));
    wheel_dir.row(2) << wheel_vector.row(2).cross(wheel_normal.row(2));

//     var coeff_matrix = wheel_vectors.map((a,i) => 
            // [a[1] * wheel_direction_vectors[i][2] - a[2] * wheel_direction_vectors[i][1], 
            //  a[2] * wheel_direction_vectors[i][0] - a[0] * wheel_direction_vectors[i][2], 
            //  a[0] * wheel_direction_vectors[i][1] - a[1] * wheel_direction_vectors[i][0]])
    Matrix3d wheel_motion;
    wheel_motion.row(0) = wheel_vector.row(0).cross(wheel_dir.row(0)).normalized();
    wheel_motion.row(1) = wheel_vector.row(1).cross(wheel_dir.row(1)).normalized();
    wheel_motion.row(2) = wheel_vector.row(2).cross(wheel_dir.row(2)).normalized();

//     coeff_matrix = coeff_matrix.map((x) => [-x[0], x[1], -x[2]])
    Matrix3d m;
    m <<    -wheel_motion(0, 0), wheel_motion(0, 1), -wheel_motion(0, 2),
            -wheel_motion(1, 0), wheel_motion(1, 1), -wheel_motion(1, 2),
            -wheel_motion(2, 0), wheel_motion(2, 1), -wheel_motion(2, 2);

    return m;
};

void printMatrix3d(Matrix3d& m, const char* name)
{
    Serial.printf(" %s = ", name);
    for(int r=0; r<3; r++)
    {
        Serial.printf("\t%0.8f %0.8f %0.8f\n", m(r,0), m(r,1), m(r,2));
    };
};
