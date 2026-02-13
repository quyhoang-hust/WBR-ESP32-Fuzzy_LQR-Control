#include "MotorBlock.h"

#define MAX_VOLTAGE 12.0
#define DEADZONE    20
#define PWM_FREQ    5000
#define PWM_RES     8

void setMot(int pin_rpwm, int pin_lpwm, float v) {
  if (v > MAX_VOLTAGE) v = MAX_VOLTAGE;
  if (v < -MAX_VOLTAGE) v = -MAX_VOLTAGE;

  int pwm = (int)(abs(v) / MAX_VOLTAGE * 255.0);
  
  if (pwm < DEADZONE) {
    ledcWrite(pin_rpwm, 0); 
    ledcWrite(pin_lpwm, 0); 
    return;
  }
  
  if (v > 0) { 
    ledcWrite(pin_rpwm, pwm); 
    ledcWrite(pin_lpwm, 0); 
  }
  else { 
    ledcWrite(pin_rpwm, 0); 
    ledcWrite(pin_lpwm, pwm); 
  }
}

void TaskMotor(void *pvParameters) {
  // ESP32 Core v3.0+
  ledcAttach(PIN_RPWM_L, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_LPWM_L, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_RPWM_R, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_LPWM_R, PWM_FREQ, PWM_RES);

  MotorCommand cmd;

  for (;;) {
    if (xQueueReceive(queueMotor, &cmd, portMAX_DELAY)) {
      setMot(PIN_RPWM_L, PIN_LPWM_L, cmd.voltageL);
      setMot(PIN_RPWM_R, PIN_LPWM_R, cmd.voltageR);
    }
  }
}
