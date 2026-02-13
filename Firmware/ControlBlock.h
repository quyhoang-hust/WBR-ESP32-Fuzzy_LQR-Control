#ifndef CONTROL_BLOCK_H
#define CONTROL_BLOCK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "SensorBlock.h"

struct MotorCommand {
  float voltageL;
  float voltageR;
};

extern QueueHandle_t queueMotor;
void TaskController(void *pvParameters);

#endif