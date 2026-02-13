#include "ControlBlock.h"
#include "PostureBlock.h" // Để truy cập servoAngle

// --- THÔNG SỐ KHÔNG ĐỔI ---
const float V_MIN_KICK = 0.8; 
const float V_MAX = 10.0;

// ====== FUZZY LOGIC ======
// Tính trọng số membership cho 3 tập fuzzy (low, mid, high)
// Tam giác: low(115), mid(135), high(155)
void fuzzyMembership(int angle, float &wLow, float &wMid, float &wHigh) {
  wLow = wMid = wHigh = 0.0f;

  if (angle <= 115) {
    wLow = 1.0f;
  } else if (angle <= 135) {
    wLow = (135.0f - angle) / 20.0f;
    wMid = (angle - 115.0f) / 20.0f;
  } else if (angle <= 155) {
    wMid = (155.0f - angle) / 20.0f;
    wHigh = (angle - 135.0f) / 20.0f;
  } else {
    wHigh = 1.0f;
  }
}

// Defuzzification: output = wLow*low + wMid*mid + wHigh*high
float fuzzyOutput(float wLow, float wMid, float wHigh, float low, float mid,
                  float high) {
  return wLow * low + wMid * mid + wHigh * high;
}
// ============================

void TaskController(void *pvParameters) {
  RobotState state;
  MotorCommand cmd;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // 100Hz

    // 1. Lấy dữ liệu an toàn
    if (xSemaphoreTake(xStateMutex, 5) == pdTRUE) {
      state = robotState;
      xSemaphoreGive(xStateMutex);
    } else {
      continue;
    }

    // 2. FUZZY: Tính TARGET_ANGLE + K
    float wLow, wMid, wHigh;
    fuzzyMembership(servoAngle, wLow, wMid, wHigh);

    float target = fuzzyOutput(wLow, wMid, wHigh, 12.5, 13.0, 12.5); 
    float K_THETA_F = fuzzyOutput(wLow, wMid, wHigh, 50.0, 60.0, 70.0);
    float K_THETA_DOT_F = fuzzyOutput(wLow, wMid, wHigh, 0.0, 0.0, 3.0);
    float K_X_F = fuzzyOutput(wLow, wMid, wHigh, 0.0, 0.0, 0.1);
    float K_X_DOT_F = fuzzyOutput(wLow, wMid, wHigh, 0.0, 0.0, 1.0);

    // 3. TÍNH LQR
    float theta_error = state.theta - (target * PI / 180.0);

    float u =
        -(K_THETA_F * theta_error + K_THETA_DOT_F * (state.theta_dot + 0.03) +
          K_X_F * state.x + K_X_DOT_F * state.x_dot);

    // 4. BÙ MA SÁT
    float u_out = 0;
    if (u > 0)
      u_out = u + V_MIN_KICK;
    if (u < 0)
      u_out = u - V_MIN_KICK;

    // 5. Saturation
    if (u_out > V_MAX)
      u_out = V_MAX;
    if (u_out < -V_MAX)
      u_out = -V_MAX;

    // Deadzone
    if (abs(theta_error) < 0.005)
      u_out = 0;

    // Lưu u_out để debug
    if (xSemaphoreTake(xStateMutex, 2) == pdTRUE) {
      robotState.u_out = u_out;
      xSemaphoreGive(xStateMutex);
    }

    cmd.voltageL = u_out;
    cmd.voltageR = u_out;
    xQueueOverwrite(queueMotor, &cmd);
  }
}
