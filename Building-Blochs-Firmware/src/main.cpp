#include <Arduino.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>

#include "config.h"
#include "log.h"
#include "bloch.h"

BlochSphere bloch;

using namespace Eigen;

void setup()
{    
    Serial.begin(460800);
    Serial0.begin(460800);
    // Serial1.begin(115200);
    // Serial2.begin(115200);
    // USBSerial.begin(115200);
    Serial.printf("Serial\n");
    Serial.printf("Serial0\n");
    // Serial1.printf("Serial1");
    // Serial1.printf("Serial2");
    // USBSerial.printf("USBSerial");

    Wire.begin(M5_STEPMOTORDRIVER_SDA, M5_STEPMOTORDRIVER_SCL, 400000);

    bloch.begin();

    DBG("Setup done.");
};

void loop()
{
    bloch.loop();

    time_t now = millis();
    static time_t last = 0;
    if(now - last > 5000)
    {
        DBG("queue 0.5 * Z");
        bloch.rotate(Vector3d(1, 0, 0), STEPS_PER_ROTATION_Z * 0.5);
        bloch.rotate(Vector3d(0, 1, 0), STEPS_PER_ROTATION_Z * 0.5);
        bloch.rotate(Vector3d(0, 0, 1), STEPS_PER_ROTATION_Z * 0.5);
        last = now;
    };
};

void processButtons()
{
            // update_rotation(VECTOR_X, HALF_ROTATION_X);
            // update_rotation(VECTOR_Y, HALF_ROTATION_Y);
            // update_rotation(VECTOR_Z, 0.5*HALF_ROTATION_Z);
            // update_rotation(VECTOR_H, HALF_ROTATION_H);
};
