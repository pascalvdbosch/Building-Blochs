#include <Arduino.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>

#include "config.h"
#include "log.h"
#include "bloch.h"

BlochSphere bloch;


// MOVEMENT
using namespace Eigen;
Matrix3d MotorMapping; // initialized in setup()
Vector3d update;

void setup()

    Wire.begin(M5_STEPMOTORDRIVER_SDA, M5_STEPMOTORDRIVER_SCL, 400000);

    bloch.begin();

    DBG("Setup done.");
};

void loop()
{
    bloch.loop();

    static bool once = true;
    if(once && millis() > 1000)
    {
        DBG("queue 0.5 * Z");
        bloch.rotate(Vector3d(0, 0, 1), STEPS_PER_ROTATION_Z * 0.5);
        once = false;
    };
};

void processButtons()
{
            // update_rotation(VECTOR_X, HALF_ROTATION_X);
            // update_rotation(VECTOR_Y, HALF_ROTATION_Y);
            // update_rotation(VECTOR_Z, 0.5*HALF_ROTATION_Z);
            // update_rotation(VECTOR_H, HALF_ROTATION_H);
};
