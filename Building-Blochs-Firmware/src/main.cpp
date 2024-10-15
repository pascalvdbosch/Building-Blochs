#include <Arduino.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>

#include "config.h"
#include "log.h"
#include "bloch.h"
#include "gui.h"

#include "m5stack-data.h"

BlochSphere bloch;

using namespace Eigen;

void setup()
{    
    Serial.begin(115200);
    Serial0.begin(115200);
    Serial1.begin(115200);
    Serial2.begin(115200);
    // USBSerial.begin(115200);
    // Wait until serial is enumerated
    while(millis() < 2500);
    Serial.printf("Serial\n");
    Serial0.printf("Serial0\n");
    Serial1.printf("Serial1\n");
    Serial2.printf("Serial2\n");
    // USBSerial.printf("USBSerial");

    gui_begin();

    bloch.begin();

    bloch.rotate(Vector3d(0, 0, 1), STEPS_PER_ROTATION);
    
    DBG("Setup done.");
};

void loop()
{
    gui_loop();

    bloch.loop();
};

