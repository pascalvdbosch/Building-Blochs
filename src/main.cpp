#include <Arduino.h>
#include <NeoPixelBus.h>
#include <AccelStepper.h>
#include "config.h"
#include <ArduinoEigenDense.h>
#include <NeoPixelAnimator.h>

#include "bloch.h"
#include "keys.h"

using namespace Eigen;

uint32_t scan_keys();

TaskHandle_t motorTask;

// LEDS
const uint16_t TailLength = 6; // length of the tail, must be shorter than PixelCount
const float MaxLightness = 0.5f; // max lightness at the head of the tail (0.5f is full bright)
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);
NeoGamma<NeoGammaTableMethod> colorGamma;
NeoPixelAnimator animations(1); // NeoPixel animation management object
bool animationRunning = false;

RgbColor red(LED_COLOR_SATURATION, 0, 0);
RgbColor green(0, LED_COLOR_SATURATION, 0);
RgbColor blue(0, 0, LED_COLOR_SATURATION);
RgbColor white(LED_COLOR_SATURATION);
RgbColor yellow(LED_COLOR_SATURATION, LED_COLOR_SATURATION/2,0);
RgbColor black(0);
RgbColor warmwhite(LED_COLOR_SATURATION*0.49, LED_COLOR_SATURATION*0.47, LED_COLOR_SATURATION*0.4);

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

// MOVEMENT
Matrix3d MotorMapping; // initialized in setup()
Vector3d update;
bool busy = false;
bool ready = false;

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

void update_rotation(Vector3d axis, const double angle)
{
    update = MotorMapping * axis * angle;
    ready = true;
};

void setup()
{
    Serial.begin(115200);

    strip.Begin();
    strip.Show();

    stepper1.setMaxSpeed(MAX_SPEED);
    stepper1.setAcceleration(MAX_ACCELERATION);
    stepper1.moveTo(0);

    stepper2.setMaxSpeed(MAX_SPEED);
    stepper2.setAcceleration(MAX_ACCELERATION);
    stepper2.moveTo(0);

    stepper3.setMaxSpeed(MAX_SPEED);
    stepper3.setAcceleration(MAX_ACCELERATION);
    stepper3.moveTo(0);

    pinMode(BUT_RED, INPUT_PULLUP);
    pinMode(BUT_GREEN, INPUT_PULLUP);
    pinMode(BUT_BLUE, INPUT_PULLUP);
    pinMode(BUT_YELLOW, INPUT_PULLUP);

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

    // Generate Movement Matrix
    MotorMapping = generateMotorMatrix(MOTOR_ANGLE_DEG, BASE_Z_ROTATION_DEG);
    printMatrix3d(MotorMapping, "MotorMapping");

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
    static Vector3d position(0,0,0);

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
    };

    if (ready)
    {
        busy = true;
        digitalWrite(STP_EN, LOW);//motors on
        delay(30);

        position += update;
        
        double max_update = max(max(abs(update[0]), abs(update[1])), abs(update[2]));
        //FIXME: instead of max() use the norm of the vector here and for speed
        //  or maybe the inner product of the update for the axis
        stepper1.setAcceleration(update[2] / max_update * MAX_ACCELERATION);
        stepper2.setAcceleration(update[1] / max_update * MAX_ACCELERATION);
        stepper3.setAcceleration(update[0] / max_update * MAX_ACCELERATION);

        stepper1.setMaxSpeed(update[2] / max_update * MAX_SPEED);
        stepper2.setMaxSpeed(update[1] / max_update * MAX_SPEED);
        stepper3.setMaxSpeed(update[0] / max_update * MAX_SPEED);

        // done with update
        ready = false;

        stepper1.moveTo(position[2]);
        stepper2.moveTo(position[1]);
        stepper3.moveTo(position[0]);
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
    // if busy, skip key handling
    if(busy)
        return;

    // read keys and convert to event
    keypress_t key = static_cast<keypress_t>(key2event(scan_keys()));

    switch(key)
    {
        case KEY_RED:
        {
            if (!animationRunning)
            {
                drawAllLeds(black);
                DrawTailPixels(HUE_RED);
                animations.StartAnimation(0, 66, LoopAnimUpdate); 
                animationRunning = true;
            };
        }; break;
        case KEY_GREEN:
        {
            if (!animationRunning)
            {
                drawAllLeds(black);
                DrawTailPixels(HUE_GREEN);
                animations.StartAnimation(0, 66, LoopAnimUpdate); 
                animationRunning = true;
            };
        }; break;
        case KEY_BLUE:
        {
            if (!animationRunning)
            {
                drawAllLeds(black);
                DrawTailPixels(HUE_BLUE);
                animations.StartAnimation(0, 66, LoopAnimUpdate); 
                animationRunning = true;
            };
        }; break;
        case KEY_YELLOW:
        {
            if (!animationRunning)
            {
                drawAllLeds(black);
                DrawTailPixels(HUE_YELLOW);
                animations.StartAnimation(0, 66, LoopAnimUpdate); 
                animationRunning = true;
            };
        }; break;
        case KEY_NONE:
        {
            if(animationRunning)
            {
                drawAllLeds(black);
                animationRunning = false;
            };
        }; break;

        case KEY_RED_SHORT:
        {
            // Serial.printf("KEY_RED_SHORT\n");            
            animationRunning = false;
            drawAllLeds(red);
            update_rotation(VECTOR_X, HALF_ROTATION_X);
        }; break;
        case KEY_GREEN_SHORT:
        {
            // Serial.printf("KEY_RED_SHORT\n");            
            animationRunning = false;
            drawAllLeds(red);
            update_rotation(VECTOR_Y, HALF_ROTATION_Y);
        }; break;
        case KEY_BLUE_SHORT:
        {
            // Serial.printf("KEY_RED_SHORT\n");            
            animationRunning = false;
            drawAllLeds(red);
            update_rotation(VECTOR_Z, 0.5*HALF_ROTATION_Z);
        }; break;
        case KEY_YELLOW_SHORT:
        {
            // Serial.printf("KEY_RED_SHORT\n");            
            animationRunning = false;
            drawAllLeds(red);
            update_rotation(VECTOR_H, HALF_ROTATION_H);
        }; break;
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
