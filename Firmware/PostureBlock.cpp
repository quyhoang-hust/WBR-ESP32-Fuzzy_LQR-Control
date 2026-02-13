#include "PostureBlock.h"

Servo servo1;
Servo servo2;

int servoAngle = 155;      
int targetServoAngle = 155;

void setServoAngle(Servo &sv, int angle) {
  if (angle < 0)
    angle = 0;
  if (angle > MAX_ANGLE)
    angle = MAX_ANGLE;
  int us = map(angle, 0, MAX_ANGLE, MIN_US, MAX_US);
  sv.writeMicroseconds(us);
}

void setupPosture() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  // KHÔNG allocate timer 2, 3 ở đây!

  servo1.setPeriodHertz(50);
  servo1.attach(PIN_SERVO_1, MIN_US, MAX_US);

  servo2.setPeriodHertz(50);
  servo2.attach(PIN_SERVO_2, MIN_US, MAX_US);

  int angle1 = servoAngle;
  int angle2 = MAX_ANGLE - servoAngle;

  setServoAngle(servo1, angle1);
  setServoAngle(servo2, angle2);
}

void TaskPosture(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50)); // 50ms = 20Hz
    if (servoAngle < targetServoAngle) {
      servoAngle++;
    } else if (servoAngle > targetServoAngle) {
      servoAngle--;
    }

    int angle1 = servoAngle;
    int angle2 = MAX_ANGLE - servoAngle;
    setServoAngle(servo1, angle1);
    setServoAngle(servo2, angle2);
  }
}
