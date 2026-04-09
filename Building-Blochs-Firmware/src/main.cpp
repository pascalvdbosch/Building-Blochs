#include <Arduino.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>

#include "log.h"
#include "bloch.h"
#include "gui.h"
#include "receiver.h"

#include "m5stack-data.h"

BlochSphere bloch;

using namespace Eigen;

void setup()
{
    Serial.begin(115200);
    // Wait until serial is enumerated
    Serial.printf("Serial\n");
    // USBSerial.printf("USBSerial");

    Serial.printf("receiver_setup\n");

    receiver_setup();

    Serial.printf("gui_begin\n");

    gui_begin();

    Serial.printf("bloch.begin\n");

    bloch.begin();

    Serial.printf("initial rotate\n");

    bloch.rotate(Vector3d(0, 0, 1), STEPS_PER_ROTATION);

    DBG("Setup done.");
};

void loop()
{
    gui_loop();
    receiver_loop();

    bloch.loop();
};
