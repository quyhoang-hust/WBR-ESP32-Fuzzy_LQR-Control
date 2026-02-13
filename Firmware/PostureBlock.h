#ifndef POSTURE_BLOCK_H
#define POSTURE_BLOCK_H

#include "SensorBlock.h"
#include <Arduino.h>
#include <ESP32Servo.h>

#define PIN_SERVO_1 18 // Servo Phải
#define PIN_SERVO_2 19 // Servo Trái

#define MIN_US 500
#define MAX_US 2500
#define MAX_ANGLE 260

#define FIXED_ANGLE 155

extern int servoAngle;      
extern int targetServoAngle; 

void setupPosture();
void TaskPosture(void *pvParameters);

#endif