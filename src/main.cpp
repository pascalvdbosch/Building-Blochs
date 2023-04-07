#include <Arduino.h>
#include <NeoPixelBus.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>

#include "keys.h"

using namespace Eigen;

uint32_t scan_keys();

TaskHandle_t motorTask;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);

RgbColor red(LED_COLOR_SATURATION, 0, 0);
RgbColor green(0, LED_COLOR_SATURATION, 0);
RgbColor blue(0, 0, LED_COLOR_SATURATION);
RgbColor white(LED_COLOR_SATURATION);
RgbColor yellow(LED_COLOR_SATURATION,LED_COLOR_SATURATION/2,0);
RgbColor black(0);

AccelStepper stepper1(AccelStepper::DRIVER, STPA_STEP, STPA_DIR);
AccelStepper stepper2(AccelStepper::DRIVER, STPB_STEP, STPB_DIR);
AccelStepper stepper3(AccelStepper::DRIVER, STPC_STEP, STPC_DIR);

Matrix3d MotorMapping; // initialized in setup()
double pos[3] {0, 0, 0};
double HALF_ROTATION = 0; // initialized in setup() as it need MotorMapping
const double MAX_SPEED(20000.0);
const double ACCELERATION(5000.0);
int iteration = 0;
bool busy = false;
bool ready = false;
bool write_done = false;
Vector3d update;

void update_rotation(Vector3d axis, const double angle)
{
    update = MotorMapping * axis * angle;
    ready = true;
};

/* CORE 1 */
void motorCall(void * parameter)
{
    for(;;){
        if (stepper1.distanceToGo() == 0 && stepper2.distanceToGo() == 0 && stepper3.distanceToGo() == 0)
        {
            double max_update = max(max(abs(update[0]), abs(update[1])), abs(update[2]));
            if (busy)
            {
                busy = false;
                digitalWrite(STP_EN, HIGH); //motors off
                write_done = true;
            }
            if (ready)
            {
                busy = true;
                digitalWrite(STP_EN, LOW);//motors on
                delay(30);

                pos[0] += update[0];
                pos[1] += update[1];
                pos[2] += update[2];

                ready = false;
                
                stepper1.setAcceleration(update[2] / max_update * ACCELERATION);
                stepper2.setAcceleration(update[1] / max_update * ACCELERATION);
                stepper3.setAcceleration(update[0] / max_update * ACCELERATION);

                stepper1.setMaxSpeed(update[2] / max_update * MAX_SPEED);
                stepper2.setMaxSpeed(update[1] / max_update * MAX_SPEED);
                stepper3.setMaxSpeed(update[0] / max_update * MAX_SPEED);

                stepper1.moveTo(pos[2]);
                stepper2.moveTo(pos[1]);
                stepper3.moveTo(pos[0]);

                update[0] = 0;
                update[1] = 0;
                update[2] = 0;
            }
        }
        else
        {
            stepper1.run();
            stepper2.run();
            stepper3.run();
        }
    }
}

void setup()
{
    Serial.begin(115200);

    strip.Begin();
    strip.Show();

    stepper1.setMaxSpeed(20000.0);
    stepper1.setAcceleration(5000.0);
    stepper1.moveTo(0);

    stepper2.setMaxSpeed(20000.0);
    stepper2.setAcceleration(5000.0);
    stepper2.moveTo(0);

    stepper3.setMaxSpeed(20000.0);
    stepper3.setAcceleration(5000.0);
    stepper3.moveTo(0);

    pinMode(STP_EN, OUTPUT);
    pinMode(STP_MS1, OUTPUT);
    pinMode(STP_MS2, OUTPUT);
    pinMode(STP_MS2, OUTPUT);

    digitalWrite(STP_EN, HIGH);     //enable pin,high is disabled motors
    digitalWrite(STP_MS1, HIGH);    //eights microstepping seems ideal = HIGH HIGH LOW
    digitalWrite(STP_MS2, HIGH);
    digitalWrite(STP_MS3, LOW);

    digitalWrite(STP_EN, LOW);//motors on to align coils
    delay(100);
    digitalWrite(STP_EN, HIGH); //motors off

    pinMode(BUT_RED, INPUT_PULLUP);
    pinMode(BUT_GREEN, INPUT_PULLUP);
    pinMode(BUT_BLUE, INPUT_PULLUP);
    pinMode(BUT_YELLOW, INPUT_PULLUP);

    // Generate Movement Matrix
    // MotorMapping = generateMotorMatrix(28.5, 30);
    // Generated with block.js with angle = 28.5 and z_rot = PI/6
    MotorMapping << 0.76107794, -0.43940856, 0.47715876,
                -0.76107794, -0.43940856, 0.47715876,
                -0.00000000, 0.87881711, 0.47715876;
    // HALF_ROTATION = 9450 / MotorMapping(2, 1); //for 250mm ball
    HALF_ROTATION = 3930; // for 149mm ball

    // Serial.println("MotorMapping = ");
    // Serial.printf("\t%f %f %f\n", MotorMapping(0,0), MotorMapping(0,1), MotorMapping(0,2));
    // Serial.printf("\t%f %f %f\n", MotorMapping(1,0), MotorMapping(1,1), MotorMapping(1,2));
    // Serial.printf("\t%f %f %f\n", MotorMapping(2,0), MotorMapping(2,1), MotorMapping(2,2));

    disableCore0WDT(); //I disable the core becasue i dont have the WDT reset functions working yet
    xTaskCreatePinnedToCore(motorCall, "motorTask", 1000, NULL, 1, &motorTask, 0);
};

void loop()
{
    // if busy, skip key handling
    if(busy)
        return;

    // read keys and convert to event
    keypress_t key = static_cast<keypress_t>(key2event(scan_keys()));

    const Vector3d VECTOR_X(1,0,0);
    const Vector3d VECTOR_Y(0,1,0);
    const Vector3d VECTOR_Z(0,0,1);
    const Vector3d VECTOR_H(sqrt(2)/2, 0, sqrt(2)/2);
    // const Vector3d VECTOR_H(.690, 0, .750);
    switch(key)
    {
        case KEY_RED:
        {
            // Serial.printf("KEY_RED\n");
            // if(not_in_cucle_led)
            //     cycle_led(red)
        }; break; 
        case KEY_RED_SHORT:
        {
            Serial.printf("KEY_RED_SHORT\n");
            for (uint32_t i = 0; i < LED_COUNT; i++)
            {
                strip.SetPixelColor(i,red);
            };
            strip.Show();

            update_rotation(VECTOR_X, HALF_ROTATION);
        }; break;
        case KEY_RED_LONG:
        {
            Serial.printf("KEY_RED_LONG\n");
            // longkey = KEY_RED;
        }; break;
        case KEY_RED_LONG_REPEAT:
        {
            Serial.printf("KEY_RED_LONG_REPEAT\n");
        }; break;

        case KEY_LONG_RELEASED:
            Serial.printf("KEY_RELEASED");
        case KEY_NONE:
        {
            // if(cycling_leds)
            //   cycle_led(none)
        }; break;

        case KEY_GREEN_SHORT:
        {
            Serial.printf("KEY_GREEN_SHORT\n");
            for (uint32_t i = 0; i < LED_COUNT; i++)
            {
                strip.SetPixelColor(i,green);
            };
            strip.Show();
            update_rotation(VECTOR_Y, HALF_ROTATION);
        }; break;

        case KEY_BLUE_SHORT:
        {
            Serial.printf("KEY_BLUE_SHORT\n");
            for (uint32_t i = 0; i < LED_COUNT; i++)
            {
                strip.SetPixelColor(i,blue);
            };
            strip.Show();
            update_rotation(VECTOR_Z, HALF_ROTATION * 0.548);
        }; break;
    
        case KEY_YELLOW_SHORT:
        {
            Serial.printf("KEY_YELLOW_SHORT\n");
            for (uint32_t i = 0; i < LED_COUNT; i++)
            {
                strip.SetPixelColor(i, yellow);
            };
            strip.Show();
            update_rotation(VECTOR_H, HALF_ROTATION);
        }; break;

        default: break;
    };
};

uint32_t scan_keys()
{
	// Read current states
	uint32_t pressed = KEY_NONE;
	if(digitalRead(BUT_RED) == LOW)
		pressed |= KEY_RED;
	if(digitalRead(BUT_GREEN) == LOW)
		pressed |= KEY_GREEN;
	if(digitalRead(BUT_BLUE) == LOW)
		pressed |= KEY_BLUE;
	if(digitalRead(BUT_YELLOW) == LOW)
		pressed |= KEY_YELLOW;
	return pressed;
};
