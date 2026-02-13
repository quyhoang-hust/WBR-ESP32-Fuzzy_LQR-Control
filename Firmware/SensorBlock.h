#ifndef SENSOR_BLOCK_H
#define SENSOR_BLOCK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Cấu hình phần cứng
#define MPU_ADDR 0x68
#define ENC_L_A 34
#define ENC_L_B 35
#define ENC_R_A 39
#define ENC_R_B 36

// Struct dữ liệu
struct RobotState {
  float theta;
  float theta_dot;
  float x;
  float x_dot;
  float height;
  float u_out; // Tín hiệu điều khiển (để debug)
};

// Dùng extern để báo hiệu biến này nằm ở file .ino
extern SemaphoreHandle_t xStateMutex;
extern RobotState robotState;

void TaskSensor(void *pvParameters);
#endif
