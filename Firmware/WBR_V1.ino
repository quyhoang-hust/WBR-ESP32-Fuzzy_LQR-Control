#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include "SensorBlock.h"
#include "PostureBlock.h"
#include "ControlBlock.h"
#include "MotorBlock.h"
#include "DebugBlock.h"

// --- KHAI BÁO BIẾN TOÀN CỤC ---
SemaphoreHandle_t xStateMutex; 
QueueHandle_t queueMotor;     
RobotState robotState;        

void setup() {
  Serial.begin(115200);
  Serial.println("--- WRB SYSTEM STARTING ---");

  //Khởi tạo RTOS
  xStateMutex = xSemaphoreCreateMutex();
  queueMotor = xQueueCreate(1, sizeof(MotorCommand));

  setupPosture();

  //Khởi tạo giá trị ban đầu
  robotState.height = 0.16;
  robotState.theta = 0.0;
  robotState.x = 0.0;
  robotState.theta_dot = 0.0;
  robotState.x_dot = 0.0;

  //TẠO TASK (ĐÃ BẬT TẤT CẢ)
  
  // Core 1
  xTaskCreatePinnedToCore(TaskSensor, "Sens", 4096, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(TaskController, "Ctrl", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(TaskMotor, "Mot", 2048, NULL, 2, NULL, 1);

  // Core 0
  xTaskCreatePinnedToCore(TaskDebug, "Dbg", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskPosture, "Post", 2048, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelete(NULL);
}