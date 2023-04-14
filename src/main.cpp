#include <Arduino.h>
#include <NeoPixelBus.h>
#include <AccelStepper.h>
#include "config.h"
#include <ezButton.h>
#include <NeoPixelAnimator.h>

// LEDS
const uint16_t TailLength = 6; // length of the tail, must be shorter than PixelCount
const float MaxLightness = 0.5f; // max lightness at the head of the tail (0.5f is full bright)
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);
NeoGamma<NeoGammaTableMethod> colorGamma;
NeoPixelAnimator animations(1); // NeoPixel animation management object

RgbColor red(LED_COLOR_SATURATION, 0, 0);
RgbColor green(0, LED_COLOR_SATURATION, 0);
RgbColor blue(0, 0, LED_COLOR_SATURATION);
RgbColor white(LED_COLOR_SATURATION);
RgbColor yellow(LED_COLOR_SATURATION*1.5,LED_COLOR_SATURATION/3,0);
RgbColor black(0);
RgbColor warmwhite(LED_COLOR_SATURATION*0.49, LED_COLOR_SATURATION*0.47, LED_COLOR_SATURATION*0.4);

// BUTTONS
ezButton buttonRed(BUT_RED);
ezButton buttonGreen(BUT_GREEN);
ezButton buttonBlue(BUT_BLUE);
ezButton buttonYellow(BUT_YELLOW);

// Motors
AccelStepper stepper1(AccelStepper::DRIVER, STPA_STEP, STPA_DIR);
AccelStepper stepper2(AccelStepper::DRIVER, STPB_STEP, STPB_DIR);
AccelStepper stepper3(AccelStepper::DRIVER, STPC_STEP, STPC_DIR);

// Functions
void processLeds();
void processButtons();
void processMotors();
void LoopAnimUpdate(const AnimationParam& param);
void drawAllLeds(RgbColor colorToDraw);
void DrawTailPixels(float hue);

/*
//coeffs for 35 degrees
const double coeffs[3][3] = {                          //reverse engineering Tjeerd
    {0.81915204,  -0.,         0.57357644},            //cos(35),0,sin(35)
    {-0.40957602, -0.70940648, 0.57357644},            //-cos(35)/2,-(sqrt(3)/2)*cos(35),sin(35)
    {-0.40957602,  0.70940648, 0.57357644}             //-cos(35)/2,(sqrt(3)/2)*cos(35),sin(35)
};*/

//coeffs for 28.5 degrees
// const double coeffs[3][3] = {
//     {0.87881711, 0.00000000, 0.47715876},
//     {-0.43940856, -0.76107794, 0.47715876},
//     {-0.43940856, 0.76107794, 0.47715876}
// };

// Generated with block.js with angle = 28.5 and z_rot = PI/6
const double coeffs[3][3] = {
    {0.76107794, -0.43940856, 0.47715876},
    {-0.76107794, -0.43940856, 0.47715876},
    {-0.00000000, 0.87881711, 0.47715876}
};

double pos[3] {0.};
const double HALF_ROTATION_X(4050); //RED, for 149mm balL
const double HALF_ROTATION_Y(3945); //GREEN, for 149mm ball
const double HALF_ROTATION_Z(4275); //BLUE=0.5z, for 149mm ball
const double HALF_ROTATION_H(4130); //for 149mm ball
const double MAX_SPEED(20000.0);
const double ACCELERATION(5000.0);
int iteration = 0;
bool busy = false;
bool ready = false;
bool write_done = false;
double update[3] = {0.};
bool animationRunning = false;

void LoopAnimUpdate(const AnimationParam& param)
{
    // wait for this animation to complete,
    // we are using it as a timer of sorts
    if (param.state == AnimationState_Completed)
    {
        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);

        // rotate the complete strip one pixel to the right on every update
        strip.RotateRight(1);
    }
};

void drawAllLeds(RgbColor colorToDraw){
    for (uint32_t i = 0; i < LED_COUNT; i++){
                strip.SetPixelColor(i,colorToDraw); 
    }
    strip.Show();
};

void DrawTailPixels(float hue)
{
    // using Hsl as it makes it easy to pick from similiar saturated colors
   
    for (uint16_t index = 0; index < strip.PixelCount() && index <= TailLength; index++)
    {
        float lightness = index * MaxLightness / TailLength;
        RgbColor color = HslColor(hue, 1.0f, lightness);

        strip.SetPixelColor(index, colorGamma.Correct(color));
    }
};

void update_positions(double (&update)[3], const double (&u)[3], const double theta)
{
    update[0] = (coeffs[0][0] * u[0] + coeffs[0][1] * u[1] + coeffs[0][2] * u[2]) * theta;
    update[1] = (coeffs[1][0] * u[0] + coeffs[1][1] * u[1] + coeffs[1][2] * u[2]) * theta;
    update[2] = (coeffs[2][0] * u[0] + coeffs[2][1] * u[1] + coeffs[2][2] * u[2]) * theta;
    ready = true;
};

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

    buttonRed.setDebounceTime(50);
    buttonBlue.setDebounceTime(50);
    buttonGreen.setDebounceTime(50);
    buttonYellow.setDebounceTime(50);

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

    drawAllLeds(warmwhite);
};

void loop()
{
    processMotors();
    processButtons(); // will set ready = true if position update needed
    processLeds();
};

void processMotors()
{
    if (stepper1.distanceToGo() || stepper2.distanceToGo() || stepper3.distanceToGo())
    {
        stepper1.run();
        stepper2.run();
        stepper3.run();

        return;
    };

    if (busy)
    {
        drawAllLeds(warmwhite);
        digitalWrite(STP_EN, HIGH); //motors off
        busy = false;
        write_done = true;
    };

    if (ready)
    {
        busy = true;
        digitalWrite(STP_EN, LOW);//motors on
        delay(30);

        pos[0] += update[0];
        pos[1] += update[1];
        pos[2] += update[2];

        ready = false;
        
        double max_update = max(max(abs(update[0]), abs(update[1])), abs(update[2]));
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
    };
};

void processLeds()
{
    if(busy)
        return;

    if(animationRunning)
    {
        animations.UpdateAnimations();
        strip.Show();
    };
};

void processButtons()
{
    buttonRed.loop();
    buttonGreen.loop();
    buttonBlue.loop();
    buttonYellow.loop();

    if(busy)
        return;

    if(buttonRed.isPressed()){
        if (!animationRunning){
            drawAllLeds(black);
            DrawTailPixels(0); //0.33 green, 0 for red, 0.667 for blue, 0.083 for yellow
            animations.StartAnimation(0, 66, LoopAnimUpdate); 
            animationRunning = true;
        }   
    }

    if(buttonRed.isReleased()){
        animationRunning = false;
        drawAllLeds(red);
        update_positions(update, (double[3]){1,0,0}, HALF_ROTATION_X);//do X
    }

    if(buttonGreen.isPressed()){
        if (!animationRunning){
            drawAllLeds(black);
            DrawTailPixels(0.33); //0.33 green, 0 for red, 0.667 for blue, 0.083 for yellow
            animations.StartAnimation(0, 66, LoopAnimUpdate); 
            animationRunning = true;
        }   
    }

    if(buttonGreen.isReleased()){
        animationRunning = false;
        drawAllLeds(green);
        update_positions(update, (double[3]){0,1,0}, HALF_ROTATION_Y);//do y
    }

    if(buttonBlue.isPressed()){
        if (!animationRunning){
            drawAllLeds(black);
            DrawTailPixels(0.667); //0.33 green, 0 for red, 0.667 for blue, 0.083 for yellow
            animations.StartAnimation(0, 66, LoopAnimUpdate); 
            animationRunning = true;
        }   
    }

    if(buttonBlue.isReleased()){
        animationRunning = false;
        drawAllLeds(blue);
        update_positions(update, (double[3]){0,0,1}, .5*HALF_ROTATION_Z);//do s, tuned this line for movement accuracy
    }

    if(buttonYellow.isPressed()){
        if (!animationRunning){
            drawAllLeds(black);
            DrawTailPixels(0.083); //0.33 green, 0 for red, 0.667 for blue, 0.083 for yellow
            animations.StartAnimation(0, 66, LoopAnimUpdate); 
            animationRunning = true;
        }   
    }

    if(buttonYellow.isReleased()){
        animationRunning = false;
        drawAllLeds(yellow);
        update_positions(update, (double[3]){.707,0,.707}, HALF_ROTATION_H);//do h, tuned this line for movement accuracy
    }
};

