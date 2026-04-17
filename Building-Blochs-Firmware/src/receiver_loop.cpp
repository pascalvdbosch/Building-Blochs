#include "receiver.h"
#include "gui.h"
#include <Arduino.h>
#include <ArduinoEigenDense.h>

#include "bloch.h"
#include "config.h"

extern BlochSphere bloch; // main.cpp

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

namespace
{
    BLEServer *pServer = nullptr;
    BLECharacteristic *pTxCharacteristic = nullptr;
    bool deviceConnected = false;
    bool oldDeviceConnected = false;
    String bleBuffer = ""; // Stores incoming text

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

    // Callback for when the phone connects/disconnects
    class MyServerCallbacks : public BLEServerCallbacks
    {
        void onConnect(BLEServer *pServer) { deviceConnected = true; };
        void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
    };

    // Callback for when the phone sends data
    class MyCallbacks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            String rxValue = pCharacteristic->getValue().c_str();
            if (rxValue.length() > 0)
            {
                bleBuffer += rxValue; // Add incoming packets to our buffer
            }
        }
    };

} // namespace

// --- NEW: Dependency-Free Quaternion Tracker ---
struct QuatTracker
{
    double w = 1.0, x = 0.0, y = 0.0, z = 0.0;

    void applyRotation(Vector3d axis, double turns)
    {
        // 1. Normalize the axis
        double ax = axis[0], ay = axis[1], az = axis[2];
        double axis_mag = sqrt(ax * ax + ay * ay + az * az);
        if (axis_mag == 0)
            return;
        ax /= axis_mag;
        ay /= axis_mag;
        az /= axis_mag;

        // 2. Convert to quaternion components
        double angle = turns * 2.0 * PI;
        double s = sin(angle / 2.0);
        double qw = cos(angle / 2.0);
        double qx = ax * s;
        double qy = ay * s;
        double qz = az * s;

        // 3. Multiply current state by new rotation (global axis rotation)
        double nw = qw * w - qx * x - qy * y - qz * z;
        double nx = qw * x + qx * w + qy * z - qz * y;
        double ny = qw * y - qx * z + qy * w + qz * x;
        double nz = qw * z + qx * y - qy * x + qz * w;

        // 4. Normalize and update state to prevent float drift
        double mag = sqrt(nw * nw + nx * nx + ny * ny + nz * nz);
        w = nw / mag;
        x = nx / mag;
        y = ny / mag;
        z = nz / mag;
    }

    void calculateReset(Vector3d &outAxis, double &outTurns)
    {
        // Inverse quaternion (conjugate)
        double inv_w = w, inv_x = -x, inv_y = -y, inv_z = -z;

        // Clamp to avoid NaN errors from acos()
        if (inv_w > 1.0)
            inv_w = 1.0;
        if (inv_w < -1.0)
            inv_w = -1.0;

        double angle = 2.0 * acos(inv_w);
        double s = sqrt(1.0 - inv_w * inv_w);

        if (s < 0.001)
        {
            outAxis = Vector3d(1, 0, 0); // Arbitrary axis, 0 turns
            outTurns = 0.0;
        }
        else
        {
            outAxis = Vector3d(inv_x / s, inv_y / s, inv_z / s);
            outTurns = angle / (2.0 * PI);
        }

        // Enforce the shortest physical path (<= 180 degrees)
        if (outTurns > 0.5)
        {
            outTurns = 1.0 - outTurns;
            // Flip axis manually to avoid missing '-' operator overload errors
            outAxis = Vector3d(-(inv_x / s), -(inv_y / s), -(inv_z / s));
        }

        // Reset the tracker to Identity
        w = 1.0;
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
};

QuatTracker netRotation;

// Helper function to execute a rotation AND track its state
void executeRotation(Vector3d axis, double turns)
{
    bloch.rotate(axis, STEPS_PER_ROTATION * turns);
    netRotation.applyRotation(axis, turns);
}

// Helper function to return to identity (0, 0, 1) in the shortest path
void receiver_reset_bloch()
{
    Vector3d resetAxis(1, 0, 0);
    double resetTurns = 0.0;

    netRotation.calculateReset(resetAxis, resetTurns);

    if (resetTurns > 0.001)
    {
        bloch.rotate(resetAxis, STEPS_PER_ROTATION * resetTurns);
    }
}

bool receiver_apply_permute(const BlochSphere::permute_t permute)
{
    switch (permute)
    {
    case BlochSphere::PERMUTE_X:
        executeRotation(Vector3d(1, 0, 0), 0.5);
        return true;
    case BlochSphere::PERMUTE_Y:
        executeRotation(Vector3d(0, 1, 0), 0.5);
        return true;
    case BlochSphere::PERMUTE_Z:
        executeRotation(Vector3d(0, 0, 1), 0.5);
        return true;
    case BlochSphere::PERMUTE_H:
        executeRotation(Vector3d(0.707106, 0, 0.707106), 0.5);
        return true;
    case BlochSphere::PERMUTE_S:
        executeRotation(Vector3d(0, 0, 1), 0.25);
        return true;
    case BlochSphere::PERMUTE_NS:
        executeRotation(Vector3d(0, 0, 1), -0.25);
        return true;
    case BlochSphere::PERMUTE_T:
        executeRotation(Vector3d(0, 0, 1), 0.125);
        return true;
    case BlochSphere::PERMUTE_NT:
        executeRotation(Vector3d(0, 0, 1), -0.125);
        return true;
    default:
        return false;
    }
}

void receiver_setup()
{
    BLEDevice::init("M5Stack_Bloch_BLE");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();
    pServer->getAdvertising()->start();
}

// Updated these to use our new tracking wrapper
void voorwaarts() { executeRotation(Vector3d(1, 0, 0), 0.5); }
void achterwaarts() { executeRotation(Vector3d(1, 0, 0), -0.5); }
void links() { executeRotation(Vector3d(0, 1, 0), 0.5); }
void rechts() { executeRotation(Vector3d(0, 1, 0), -0.5); }
void linksBoven() { executeRotation(Vector3d(0.707106, 0, 0.707106), 0.5); }
void rechtsBoven() { executeRotation(Vector3d(0.707106, 0, 0.707106), -0.5); }
void linksOnder() { executeRotation(Vector3d(0.707106, 0, -0.707106), 0.5); }
void rechtsOnder() { executeRotation(Vector3d(0.707106, 0, -0.707106), -0.5); }

void setLedRing(int r, int g, int b)
{
    // Update the entire screen background
    gui_set_screen_color(r, g, b);
}

bool pulse = false;
unsigned long pulseEnd = millis();

void receiver_loop()
{
    // Handle disconnection
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;

        gui_set_ble_connected(false); // <--- ADD THIS
    }

    // Handle connection
    if (deviceConnected && !oldDeviceConnected)
    {
        oldDeviceConnected = deviceConnected;
        
        gui_set_ble_connected(true); // <--- ADD THIS
    }

    int newlineIndex = bleBuffer.indexOf('\n');

    if (newlineIndex != -1)
    {
        String commando = bleBuffer.substring(0, newlineIndex);
        bleBuffer = bleBuffer.substring(newlineIndex + 1);
        commando.trim();

        String strs[20];
        int StringCount = 0;
        while (commando.length() > 0)
        {
            int index = commando.indexOf(',');
            if (index == -1)
            {
                strs[StringCount++] = commando;
                break;
            }
            else
            {
                strs[StringCount++] = commando.substring(0, index);
                commando = commando.substring(index + 1);
            }
        }

        for (int i = 0; i < StringCount; i++)
        {
            String task = strs[i];

            // gui_set_ble_task(task.c_str()); // <--- ADD THIS for real-time task display on GUI

            if (task == "voorwaarts")
                voorwaarts();
            else if (task == "achterwaarts")
                achterwaarts();
            else if (task == "links")
                links();
            else if (task == "rechts")
                rechts();
            else if (task == "linksboven")
                linksBoven();
            else if (task == "rechtsboven")
                rechtsBoven();
            else if (task == "linksonder")
                linksOnder();

            // Replaced raw bloch.rotate() with executeRotation() for accurate tracking
            else if (task == "Xgate")
                receiver_apply_permute(BlochSphere::PERMUTE_X);
            else if (task == "Ygate")
                receiver_apply_permute(BlochSphere::PERMUTE_Y);
            else if (task == "Zgate")
                receiver_apply_permute(BlochSphere::PERMUTE_Z);
            else if (task == "Hgate")
                receiver_apply_permute(BlochSphere::PERMUTE_H);
            else if (task == "Sgate")
                receiver_apply_permute(BlochSphere::PERMUTE_S);
            else if (task == "Tgate")
                receiver_apply_permute(BlochSphere::PERMUTE_T);
            else if (task == "S+gate")
                receiver_apply_permute(BlochSphere::PERMUTE_NS);
            else if (task == "T+gate")
                receiver_apply_permute(BlochSphere::PERMUTE_NT);

            // --- NEW: Reset Command ---
            else if (task == "reset")
                receiver_reset_bloch();

            else if (task == "led")
            {
                setLedRing(strs[i + 1].toInt(), strs[i + 2].toInt(), strs[i + 3].toInt());
                i += 3;
            }
            else if (task == "pulse")
            {
                setLedRing(strs[i + 1].toInt(), strs[i + 2].toInt(), strs[i + 3].toInt());
                pulse = true;
                pulseEnd = millis() + strs[i + 4].toInt();
                i += 4;
            }
            else if (task == "delay")
            {
                delay(strs[i + 1].toInt());
                i++;
            }

            if (deviceConnected)
            {
                task += "\n";
                pTxCharacteristic->setValue(task.c_str());
                pTxCharacteristic->notify();
            }
        }
    }

if (pulse && millis() > pulseEnd)
    {
        pulse = false;
        
        // Restore the standard background colors
        gui_reset_screen_color();
        
        // Clear the active task so the BLE overlay returns to the idle "CONNECTED" screen
        gui_set_ble_task("");
    }
}