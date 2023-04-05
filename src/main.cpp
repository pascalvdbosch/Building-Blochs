#include <Arduino.h>
#include <NeoPixelBus.h>
#include <AccelStepper.h>
#include "config.h"
#include <ezButton.h>

TaskHandle_t motorTask;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);

ezButton buttonRed(BUT_RED);
ezButton buttonGreen(BUT_GREEN);
ezButton buttonBlue(BUT_BLUE);
ezButton buttonYellow(BUT_YELLOW);

RgbColor red(LED_COLOR_SATURATION, 0, 0);
RgbColor green(0, LED_COLOR_SATURATION, 0);
RgbColor blue(0, 0, LED_COLOR_SATURATION);
RgbColor white(LED_COLOR_SATURATION);
RgbColor yellow(LED_COLOR_SATURATION,LED_COLOR_SATURATION/2,0);
RgbColor black(0);

AccelStepper stepper1(AccelStepper::DRIVER, STPA_STEP, STPA_DIR);
AccelStepper stepper2(AccelStepper::DRIVER, STPB_STEP, STPB_DIR);
AccelStepper stepper3(AccelStepper::DRIVER, STPC_STEP, STPC_DIR);

//coeffs for 35 degrees
const double coeffs[3][3] = {                          //reverse engineering Tjeerd
    {0.81915204,  -0.,         0.57357644},            //cos(35),0,sin(35)
    {-0.40957602, -0.70940648, 0.57357644},            //-cos(35)/2,-(sqrt(3)/2)*cos(35),sin(35)
    {-0.40957602,  0.70940648, 0.57357644}             //-cos(35)/2,(sqrt(3)/2)*cos(35),sin(35)
};


double pos[3] {0.};
//const double HALF_ROTATION(9450 / coeffs[2][1]); //for 250mm ball
const double HALF_ROTATION(3000 / coeffs[2][1]); //for 149mm ball
const double MAX_SPEED(20000.0);
const double ACCELERATION(5000.0);
int iteration = 0;
bool busy = false;
bool ready = false;
bool write_done = false;
double update[3] = {0.};

void update_positions(double (&update)[3], const double (&u)[3], const double theta)
{
    update[0] = (coeffs[0][0] * u[0] + coeffs[0][1] * u[1] + coeffs[0][2] * u[2]) * theta;
    update[1] = (coeffs[1][0] * u[0] + coeffs[1][1] * u[1] + coeffs[1][2] * u[2]) * theta;
    update[2] = (coeffs[2][0] * u[0] + coeffs[2][1] * u[1] + coeffs[2][2] * u[2]) * theta;
    ready = true;
}

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

    disableCore0WDT(); //I disable the core becasue i dont have the WDT reset functions working yet
    xTaskCreatePinnedToCore(motorCall, "motorTask", 1000, NULL, 1, &motorTask, 0);
}

void loop()
{
    buttonRed.loop();
    buttonGreen.loop();
    buttonBlue.loop();
    buttonYellow.loop();

    if (!busy){ 
        if(buttonRed.isPressed()){
            for (uint32_t i = 0; i < LED_COUNT; i++){
            strip.SetPixelColor(i,red); }
            strip.Show();
            update_positions(update, (double[3]){1,0,0}, HALF_ROTATION);//do X
        }

        if(buttonGreen.isPressed()){
            for (uint32_t i = 0; i < LED_COUNT; i++){
            strip.SetPixelColor(i,green);}
            strip.Show();
            update_positions(update, (double[3]){0,1,0}, HALF_ROTATION);//do y
        }

        if(buttonBlue.isPressed()){
            for (uint32_t i = 0; i < LED_COUNT; i++){
            strip.SetPixelColor(i,blue);}
            strip.Show();
            update_positions(update, (double[3]){0,0,1}, .5*HALF_ROTATION);//do s
        }

        if(buttonYellow.isPressed()){
            for (uint32_t i = 0; i < LED_COUNT; i++){
            strip.SetPixelColor(i,yellow);}
            strip.Show();
            update_positions(update, (double[3]){.707,0,.707}, HALF_ROTATION);//do h
        }
    }
}
