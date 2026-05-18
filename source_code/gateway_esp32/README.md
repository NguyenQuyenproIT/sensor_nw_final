Gateway ESP32 - README

Chức năng chính:

- Nhận JSON từ STM32 qua `Serial2`.
- Parse, chuẩn hóa dữ liệu và gọi hàm `mlp_predict()` trong `mlp_model.c`.
- Điều khiển LED cảnh báo và gửi telemetry lên ThingsBoard qua MQTT.
- Hỗ trợ RPC từ ThingsBoard để điều khiển chế độ (AI/MANUAL).

Build & upload:

```bash
platformio run --project-dir .
platformio run --target upload --project-dir .
```

Các file quan trọng:

- `src/main.cpp` — luồng runtime, Wi‑Fi, MQTT, Serial2.
- `src/mlp_model.c`, `include/mlp_model.h` — model MLP nhúng.
- `platformio.ini` — cấu hình board và libraries.

Chú ý khi cập nhật model:

- Giữ nguyên thứ tự feature và chuẩn hóa đúng mean/std.
- Nếu thay đổi tên field JSON, cập nhật parser trong `main.cpp`.
