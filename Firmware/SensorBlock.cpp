#include "SensorBlock.h"
#include <Wire.h>

#define ALPHA 0.98
#define ALPHA_GYRO 0.7 // Low-pass filter cho gyro (0.5-0.8, nhỏ = mượt hơn)
#define DT_MS 10
#define DT_S 0.01

// Thông số cơ khí: R=0.0325m, Encoder 11 xung, Hộp số 30
const float METERS_PER_COUNT = (2 * PI * 0.0325) / (11.0 * 30.0);

volatile long encCountL = 0;
volatile long encCountR = 0;
portMUX_TYPE muxEnc = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isr_EncL() {
  portENTER_CRITICAL_ISR(&muxEnc);
  if (digitalRead(ENC_L_B))
    encCountL++;
  else
    encCountL--;
  portEXIT_CRITICAL_ISR(&muxEnc);
}

void IRAM_ATTR isr_EncR() {
  portENTER_CRITICAL_ISR(&muxEnc);
  if (digitalRead(ENC_R_B))
    encCountR++;
  else
    encCountR--;
  portEXIT_CRITICAL_ISR(&muxEnc);
}

// Đọc MPU6050 đơn giản, trả về true nếu thành công
bool readMPU(int16_t *accX, int16_t *accZ, int16_t *gyrY) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) {
    return false; // I2C lỗi
  }

  uint8_t count =
      Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14, (uint8_t)true);
  if (count != 14) {
    // Clear buffer nếu có
    while (Wire.available())
      Wire.read();
    return false;
  }

  *accX = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read(); // accY - bỏ qua
  *accZ = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read(); // Temp - bỏ qua
  Wire.read();
  Wire.read(); // gyrX - bỏ qua
  *gyrY = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read(); // gyrZ - bỏ qua

  return true;
}

void initMPU6050() {
  Wire.begin(21, 22);    // SDA=21, SCL=22 (mặc định ESP32)
  Wire.setClock(400000); // 400kHz I2C

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Gyro config: ±250°/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Accel config: ±2g
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);

  Serial.println("[SENSOR] MPU6050 OK");
}

void TaskSensor(void *pvParameters) {
  initMPU6050();

  // Encoder pins - Lưu ý: GPIO 34-39 không có internal pull-up
  pinMode(ENC_L_A, INPUT);
  pinMode(ENC_L_B, INPUT);
  pinMode(ENC_R_A, INPUT);
  pinMode(ENC_R_B, INPUT);

  attachInterrupt(ENC_L_A, isr_EncL, RISING);
  attachInterrupt(ENC_R_A, isr_EncR, RISING);

  float filtered_theta = 0.0;
  long prevPosL = 0, prevPosR = 0;
  int errorCount = 0;

  // Warm-up: Đọc góc ban đầu
  vTaskDelay(pdMS_TO_TICKS(200));
  int16_t ax, az, gy;
  if (readMPU(&ax, &az, &gy)) {
    filtered_theta = -atan2((float)ax, (float)az);
    Serial.printf("[SENSOR] Init: %.1f deg\n", filtered_theta * 180.0 / PI);
  } else {
    Serial.println("[SENSOR] MPU read FAILED!");
  }

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DT_MS));

    int16_t accX, accZ, gyrY;

    if (readMPU(&accX, &accZ, &gyrY)) {
      errorCount = 0;

      float thetaAcc = -atan2((float)accX, (float)accZ);
      float thetaDot = (gyrY / 131.0f) * (PI / 180.0f); // Đã sửa dấu

      // Complementary filter
      filtered_theta = ALPHA * (filtered_theta + thetaDot * DT_S) +
                       (1.0f - ALPHA) * thetaAcc;

      // Đọc encoder
      portENTER_CRITICAL(&muxEnc);
      long currL = encCountL;
      long currR = encCountR;
      portEXIT_CRITICAL(&muxEnc);

      float x_meters = ((currL + currR) / 2.0f) * METERS_PER_COUNT;
      float x_dot = (((currL - prevPosL) + (currR - prevPosR)) / 2.0f *
                     METERS_PER_COUNT) /
                    DT_S;

      prevPosL = currL;
      prevPosR = currR;

      // Cập nhật state
      if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(2))) {
        robotState.theta = filtered_theta;
        robotState.theta_dot = thetaDot;
        robotState.x = x_meters;
        robotState.x_dot = x_dot;
        xSemaphoreGive(xStateMutex);
      }
    } else {
      // Lỗi I2C - chỉ log, không reset (để tránh treo)
      errorCount++;
      if (errorCount % 100 == 0) {
        Serial.printf("[SENSOR] I2C err x%d\n", errorCount);
      }
    }
  }
}
