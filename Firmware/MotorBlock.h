#ifndef MOTOR_BLOCK_H
#define MOTOR_BLOCK_H

#include <Arduino.h>
#include "ControlBlock.h"

#define PIN_RPWM_L 25  
#define PIN_LPWM_L 26  

#define PIN_RPWM_R 33
#define PIN_LPWM_R 32

void TaskMotor(void *pvParameters);

#endif
