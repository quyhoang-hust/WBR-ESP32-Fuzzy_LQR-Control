# 🤖 WBR with Fuzzy-LQR Controller
### Wheeled-Bipedal Technology | Advanced Control Systems

Dự án nghiên cứu và triển khai hệ thống điều khiển phản hồi trạng thái (LQR) kết hợp logic mờ (Fuzzy) cho robot lưỡng thăng có bánh (WBR) trên nền tảng ESP32.

## 🧠 Kiến trúc điều khiển (Control Architecture)
Dự án thực hiện bài toán cân bằng động cho robot bằng cách kết hợp giữa lý thuyết điều khiển hiện đại và logic mờ:

* **LQR (Linear Quadratic Regulator):** Sử dụng bộ điều khiển phản hồi trạng thái $u = -Kx$ để giữ thăng bằng. Hệ thống dựa trên vector trạng thái bao gồm: $[ \theta, \dot{\theta}, x, \dot{x} ]^T$.
* **Fuzzy Logic (Gain Scheduling):** Đóng vai trò là bộ điều chỉnh thông số thông minh. Dựa vào góc khớp chân thực tế (`servoAngle`), hệ thống thực hiện nội suy các thông số $K$ và $Target Angle$ phù hợp thông qua hàm membership tam giác (Low, Mid, High).
* **Complementary Filter (Bộ lọc bù):** Xử lý tín hiệu từ MPU6050 với hệ số $\alpha = 0.98$ để triệt tiêu nhiễu của gia tốc kế và hiện tượng trôi (drift) của con quay hồi chuyển, đảm bảo dữ liệu góc nghiêng $\theta$ luôn ổn định.

## 🏗 Kiến trúc phần mềm (Software Architecture)
Mã nguồn được thiết kế theo hướng **Modular Programming**, tận dụng tối đa khả năng xử lý song song trên **Dual-core** của ESP32 thông qua **FreeRTOS**:

### **Core 1: High Priority (Nhiệm vụ điều khiển)**
* **TaskSensor (100Hz):** Đọc cảm biến I2C, tính toán bộ lọc bù và giải mã Encoder.
* **TaskController (100Hz):** Thực thi thuật toán Fuzzy + LQR. Tính toán tín hiệu điện áp $u$ và đẩy vào hàng đợi (`Queue`).
* **TaskMotor:** Chấp hành tín hiệu PWM điều khiển driver BTS7960 với tần số 5kHz.

### **Core 0: Low Priority (Nhiệm vụ phụ trợ)**
* **TaskPosture (20Hz):** Quản lý khớp chân qua RC Servos với tính năng **Smooth Transition** (thay đổi 1° mỗi chu kỳ) để bảo vệ cơ cấu cơ khí.
* **TaskDebug:** Giao tiếp Serial, nhận lệnh thay đổi chiều cao thời gian thực và xuất dữ liệu ra Serial Plotter.

## 📺 Video vận hành thực tế
[![Xem video robot cân bằng](https://img.youtube.com/vi/h3YY7-l_Uk0/0.jpg)](https://www.youtube.com/watch?v=h3YY7-l_Uk0)
*Nhấn vào hình ảnh trên để xem video demo trên YouTube*

## 📂 Tổ chức mã nguồn
* `Firmware/`: Chứa mã nguồn chính chia theo các khối chức năng (`SensorBlock`, `ControlBlock`, `MotorBlock`,...).
* `Hardware/`: Sơ đồ nguyên lý, thiết kế khớp chân và sơ đồ đi dây.
* `Assets/`: Hình ảnh robot vận hành và kết quả mô phỏng/debug.

## 🚀 Hướng dẫn vận hành
1. Nạp code vào ESP32 thông qua Arduino IDE hoặc PlatformIO.
2. Mở **Serial Monitor/Plotter** ở baudrate `115200`.
3. Để thay đổi độ cao robot: Nhập giá trị góc từ `115` đến `155` vào Serial. Hệ thống Fuzzy sẽ tự động điều chỉnh bộ tham số $K$ tương ứng.

---
**Author:** Hoang Xuan Quy
**Project Status:** Đã cập nhật đầy đủ chức năng Fuzzy + LQR và RTOS.
