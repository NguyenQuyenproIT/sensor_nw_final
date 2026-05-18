SENSOR_NW_FINAL

Phiên bản hướng dẫn toàn diện (lý thuyết → thực tiễn) cho dự án Sensor_NW_Final.

Mục tiêu: cung cấp tài liệu đầy đủ để người đọc (kỹ sư nhúng, data scientist, devops) nắm được kiến trúc, luồng dữ liệu, cách huấn luyện MLP, xuất model sang C, triển khai firmware và vận hành hệ thống end-to-end.

1. Tổng quan hệ thống

- 2 node nhúng: STM32 (thu thập cảm biến) và ESP32 (gateway + inference + MQTT).
- Dữ liệu: ADC soil moisture, DHT22 (temp, hum) → JSON qua UART → ESP32 nhận, chuẩn hóa, gọi MLP nhúng → điều khiển LED cảnh báo và gửi telemetry lên ThingsBoard.
- MLP: huấn luyện trên CSV, lưu `mlp_model.pkl` + `scaler.pkl`, xuất sang C (`mlp_model.c`/`mlp_model.h`) để nhúng.

2. Kiến trúc & luồng dữ liệu (Data Flow)

- STM32: đọc ADC và DHT22 → build JSON: {"temp": <°C>, "hum": <%>, "soil": <%>} → gửi qua UART2 (115200, newline-terminated).
- ESP32:
  1) đọc Serial2 line-by-line, parse JSON;
  2) map fields theo thứ tự [soil, temp, hum];
  3) chuẩn hóa: x' = (x - mean) / std (dựa trên scaler từ training);
  4) forward pass MLP C nhúng → output (tanh); classify (y>0 → WARNING);
  5) điều khiển LED (active-low) và gửi telemetry qua MQTT tới ThingsBoard.

3. MLP — lý thuyết ngắn

- Mục tiêu: phân loại trạng thái đất (ví dụ NEED_WATER / OK) từ 3 feature liên tục.
- MLP (Multi-Layer Perceptron) là một mạng feed-forward nhiều lớp gồm: input layer (3), hidden layers (ví dụ 16,8), output layer (1). Kích hoạt tanh ở hidden/output giúp mapping giá trị normalized vào [-1,1].
- Chuẩn hóa (StandardScaler) rất quan trọng: đảm bảo dữ liệu runtime tuân theo phân phối training.
- Loss & training: dùng cross-entropy hoặc suitable loss cho phân loại; training dùng sklearn.MLPClassifier trong pipeline hiện tại.

4. Từ model scikit-learn sang mã C

Quy trình chính:

1) Train model trong Python → lưu `mlp_model.pkl` và `scaler.pkl`.
2) Dùng `inspect_model.py` / `read_model.py` để xuất weights, biases, mean/std.
3) Sinh `mlp_model.c`/`mlp_model.h`: biểu diễn ma trận W, vectors b, hàm scale và forward pass (tanh).
4) Kiểm thử trong môi trường desktop (C unit test) để đảm bảo kết quả giống Python (±1e-5 tolerance).

Gợi ý: giữ thứ tự feature cố định: [soil_moisture, temperature, humidity].

5. Kết nối phần cứng & wiring

- Chung GND giữa STM32 và ESP32.
- STM32 TX -> ESP32 RX (Serial2 RX pin), STM32 RX <- ESP32 TX nếu cần.
- Cấp nguồn 3.3V cho DHT22 và đảm bảo pull-up cho data line.
- LED cảnh báo active-low (LOW = ON).

6. Hướng dẫn build & deploy (step-by-step)

a) Huấn luyện model (local dev machine):

```bash
python -m venv .venv
.\.venv\Scripts\activate
pip install -r train_AI/MLP_TRAIN/requirements.txt  # hoặc pandas scikit-learn joblib numpy
python train_AI/MLP_TRAIN/train_mlp.py
```

b) Kiểm tra model & xuất trọng số:

```bash
python train_AI/MLP_TRAIN/read_model.py
python train_AI/MLP_TRAIN/inspect_model.py
```

c) Sinh `mlp_model.c`/`mlp_model.h` (nếu chưa có):

- Thực thi script xuất (nếu có), hoặc tôi có thể sinh script tự động chuyển `mlp_model.pkl` + `scaler.pkl` → C.

d) Build & flash STM32 (toolchain: Keil/STM32CubeIDE/arm-gcc):

- Mở project `source_code/soil_dht22_stm32/` → build → flash.

e) Build & upload ESP32 (PlatformIO):

```bash
platformio run --project-dir source_code/gateway_esp32
platformio run --target upload --project-dir source_code/gateway_esp32
```

Trước khi upload, cập nhật: Wi‑Fi SSID/PASSWORD, ThingsBoard token, và (nếu cần) chân UART/LED trong source_code/gateway_esp32/src/main.cpp hoặc file cấu hình.

7. Debugging checklist

- Nếu ESP32 không nhận dữ liệu: kiểm tra baudrate (115200), GND chung, newline termination, wiring TX/RX.
- Nếu DHT22 lỗi: kiểm tra pull-up 4.7K, timing bit-bang, nguồn 3.3V.
- Nếu prediction khác Python: kiểm tra scaler (mean/std), thứ tự feature, và độ chính xác số thực (float32 vs float64).
- Nếu ThingsBoard không nhận: kiểm tra token, server, MQTT topic `v1/devices/me/telemetry`.

8. Best practices & bảo mật

- Không commit secrets; dùng `secrets.h` (ignored) hoặc biến môi trường.
- Kiểm thử unit cho model C (so sánh với Python).
- Document versioning cho model: lưu model.pkl với tên có timestamp/verson.

9. README riêng cho từng thư mục (tôi đã thêm các README bên dưới) — mở rộng theo nhu cầu.

---

Nếu bạn muốn tôi sinh thêm: script export model→C, test harness C cho `mlp_model.c`, hoặc sơ đồ wiring dạng SVG, chọn mục để tôi bắt tay làm.
