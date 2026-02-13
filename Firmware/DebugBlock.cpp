#include "DebugBlock.h"
#include "PostureBlock.h"

void TaskDebug(void *pvParameters) {
  RobotState s;
  for (;;) {
    if (Serial.available()) {
      int newAngle = Serial.parseInt();
      if (newAngle >= 0 && newAngle <= 260) {
        targetServoAngle = newAngle;
        Serial.printf("[POSTURE] Target angle: %d (smooth)\n",
                      targetServoAngle);
      }
      while (Serial.available())
        Serial.read();
    }

    if (xSemaphoreTake(xStateMutex, 10)) {
      s = robotState;
      xSemaphoreGive(xStateMutex);
    }
    Serial.print("Theta:");
    Serial.print(s.theta * 180.0 / PI);
    Serial.print(" ,X_cm:");
    Serial.print(s.x * 100.0, 1);
    Serial.print(" ,Servo:");
    Serial.print(servoAngle);
    Serial.println();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}