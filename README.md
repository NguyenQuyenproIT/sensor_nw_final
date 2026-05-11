# SENSOR_NW_FINAL

Repo này là một hệ thống smart farming hoàn chỉnh theo hướng thực nghiệm: cảm biến đọc dữ liệu, STM32 xử lý và gửi lên ESP32, ESP32 chạy model AI rồi đẩy telemetry lên ThingsBoard. Phần train ML và phần firmware được tách riêng, nên người đọc có thể theo flow từng lớp mà không bị lẫn giữa dữ liệu, model và code nhúng.

## Tổng quan luồng chạy

```mermaid
flowchart LR
    A[Cảm biến đất / DHT22] --> B[STM32 đọc ADC + nhiệt độ / độ ẩm]
    B --> C[UART2 JSON: temp, hum, soil]
    C --> D[ESP32 gateway]
    D --> E[parse JSON]
    E --> F[mlp_predict(soil, temp, hum)]
    F --> G{Kết quả}
    G -->|WARNING| H[Bật LED cảnh báo]
    G -->|NORMAL| I[Tắt LED cảnh báo]
    G --> J[Telemetry ThingsBoard]
    H --> J
    I --> J
```

```mermaid
flowchart LR
    A[source_csv/ CSV dữ liệu] --> B[train_AI/Decision_Tree/train.py]
    A --> C[train_AI/MLP_TRAIN/train_mlp.py]
    B --> D[soil_model_dt.pkl]
    C --> E[mlp_model.pkl]
    C --> F[scaler.pkl]
    E --> G[ESP32 gateway]
    F --> G
```

Ý chính cần nhớ:

- `source_csv/` là nơi chứa dữ liệu nguồn và dữ liệu đã gán nhãn.
- `train_AI/` là nơi train và xuất model.
- `source_code/` là firmware nhúng.
- `gateway_esp32` là lớp trung gian giữa STM32 và cloud.

## Cấu trúc thư mục

```text
sensor_nw_final/
├── circuit_design/
├── documents/
├── source_code/
├── source_csv/
└── train_AI/
```

### `circuit_design/`

Chứa thư viện mạch:

- `DEVKIT_V1_ESP32-WROOM-32.IntLib`
- `Stm32 Blue Pill.IntLib`

Đây là tài nguyên thiết kế phần cứng, không phải source chạy trực tiếp.

### `documents/`

Thư mục ghi chú và tài liệu tham khảo cho phần ESP / MQTT / mô tả hệ thống.

### `source_csv/`

Chứa dữ liệu CSV của toàn bộ bài toán:

- `daily_temperature_extracted.csv`
- `log_temp.csv`
- `Smart_Farming_Crop_Yield_2024.csv`
- `Smart_Farming_Final_Refined.csv`
- `Smart_Farming_Smart_Labeled.csv`

### `train_AI/`

Chứa 2 pipeline train:

- `Decision_Tree/`
- `MLP_TRAIN/`

### `source_code/`

Chứa 3 phần code nhúng chính:

- `soil_moisture_stm32/`
- `soil_moisture_dht11_stm32/`
- `gateway_esp32/`

## Firmware STM32

### `source_code/soil_moisture_stm32/main.c`

File: [source_code/soil_moisture_stm32/main.c](source_code/soil_moisture_stm32/main.c)

Đây là firmware tối giản để đọc độ ẩm đất qua ADC.

Luồng xử lý:

1. Cấu hình `USART1` để in log ở `115200`.
2. Cấu hình `TIM2` làm bộ delay microsecond / millisecond.
3. Cấu hình `ADC1` trên `PA0` để đọc cảm biến độ ẩm đất.
4. Đọc 10 mẫu liên tiếp rồi lấy trung bình để giảm nhiễu.
5. Quy đổi ADC sang % độ ẩm theo công thức:

```text
moisture = (4095 - adc_value) * 100 / 4095
```

Output chính:

```text
ADC = <value> | Moisture = <percent>%
```

File này phù hợp để kiểm tra phần cứng và debug tín hiệu analog trước khi ghép vào luồng AI.

### `source_code/soil_moisture_dht11_stm32/main.c`

File: [source_code/soil_moisture_dht11_stm32/main.c](source_code/soil_moisture_dht11_stm32/main.c)

Đây là firmware dùng 2 nguồn dữ liệu:

- độ ẩm đất từ ADC `PA0`
- nhiệt độ / độ ẩm không khí từ cảm biến họ DHT trên `PB12`

Điểm cần nói rõ: tên thư mục là `dht11`, nhưng code hiện tại đang đọc theo format DHT22. Nếu phần cứng thực tế là DHT11 thì cần chỉnh lại phần đọc dữ liệu cho khớp.

Luồng xử lý:

1. Khởi tạo `USART1` để debug và `USART2` để gửi dữ liệu sang ESP32.
2. Khởi tạo `TIM2` để tạo delay chính xác cho giao tiếp bit-banging.
3. Đọc DHT trên `PB12`, kiểm tra checksum.
4. Đọc ADC đất và lấy trung bình 10 mẫu.
5. Nếu dữ liệu hợp lệ, in ra serial và gửi JSON qua UART2.

JSON được gửi sang gateway có dạng:

```json
{"temp":32.1,"hum":70.2,"soil":45}
```

Nếu đọc DHT lỗi, firmware trả về JSON lỗi:

```json
{"error":"dht"}
```

### Mối liên kết giữa 2 firmware STM32

- `soil_moisture_stm32` là bản đọc đất đơn giản để test cảm biến.
- `soil_moisture_dht11_stm32` là bản có đủ dữ liệu để đẩy sang ESP32 và chạy AI.

Nếu mục tiêu là chạy end-to-end, luồng đúng là dùng bản có DHT + soil để xuất JSON cho ESP32.

## Gateway ESP32

File chính: [source_code/gateway_esp32/src/main.cpp](source_code/gateway_esp32/src/main.cpp)

Project này dùng PlatformIO với cấu hình Arduino, MQTT và JSON:

- board: `esp32doit-devkit-v1`
- monitor speed: `115200`
- thư viện chính: `PubSubClient`, `ArduinoJson`

Luồng runtime của ESP32:

1. Kết nối Wi-Fi.
2. Kết nối MQTT / ThingsBoard.
3. Đọc từng dòng JSON từ `Serial2` nhận từ STM32.
4. Parse 3 trường bắt buộc: `temp`, `hum`, `soil`.
5. Gọi `mlp_predict((float)soil, temp, hum)`.
6. Nếu prediction > `0.0f`, gán trạng thái `WARNING` và bật LED cảnh báo.
7. Nếu không, gán trạng thái `NORMAL` và tắt LED.
8. Gửi telemetry gồm sensor data, prediction, trạng thái AI và trạng thái LED lên ThingsBoard.

Gateway hiện đang đọc đúng format JSON từ STM32 và không còn là đoạn demo rời rạc. Các trường telemetry được gửi đi gồm:

- `temperature`
- `humidity`
- `soil`
- `prediction`
- `ai_state`
- `led_state`

Lưu ý quan trọng:

- LED cảnh báo đang dùng logic active-low.
- ESP32 phụ thuộc vào model MLP đã được build sẵn trong project qua `mlp_model.h` / `mlp_model.c`.
- Nếu STM32 không gửi JSON đúng format, gateway sẽ báo `PARSE FAIL`.

## Train model

### `train_AI/Decision_Tree/train.py`

File: [train_AI/Decision_Tree/train.py](train_AI/Decision_Tree/train.py)

Mục đích của pipeline này là train cây quyết định để phân loại trạng thái tưới dựa trên 3 feature:

- `soil_moisture`
- `temperature`
- `humidity`

Luồng train:

1. Đọc CSV.
2. Kiểm tra dữ liệu đầu vào.
3. Tách `X` và `y`.
4. Chia train/test theo tỉ lệ 80/20.
5. Train `DecisionTreeClassifier`.
6. Đánh giá bằng accuracy, classification report và confusion matrix.
7. In luật cây và feature importance.
8. Lưu model ra `soil_model_dt.pkl`.

File trong folder hiện có thể dùng để đối chiếu nhanh kết quả tree và giải thích logic ra quyết định.

### `train_AI/MLP_TRAIN/train_mlp.py`

File: [train_AI/MLP_TRAIN/train_mlp.py](train_AI/MLP_TRAIN/train_mlp.py)

Đây là pipeline MLP để học quan hệ phi tuyến mạnh hơn.

Luồng train:

1. Đọc CSV.
2. Xáo trộn dữ liệu.
3. Tách `X` và `y`.
4. Chia train/test.
5. Chuẩn hóa feature bằng `StandardScaler`.
6. Train `MLPClassifier`.
7. Đánh giá model.
8. Lưu `mlp_model.pkl` và `scaler.pkl`.

Các file hỗ trợ trong cùng folder:

- [train_AI/MLP_TRAIN/read_model.py](train_AI/MLP_TRAIN/read_model.py)
- [train_AI/MLP_TRAIN/inspect_model.py](train_AI/MLP_TRAIN/inspect_model.py)

Điểm bắt buộc của MLP:

- phải đi kèm `scaler.pkl` khi suy luận
- input phải giữ đúng thứ tự `soil_moisture`, `temperature`, `humidity`

## Dữ liệu và điểm cần chú ý

### Dữ liệu đang có cột `lable`

Các file CSV nhãn hiện thấy header là:

```text
soil_moisture,temperature,humidity,lable
```

Nhưng cả 2 script train đang đọc cột `label`. Nếu bạn dùng đúng các file CSV hiện có thì cần sửa một trong hai bên cho khớp, nếu không training sẽ lỗi ngay lúc tách target.

### `source_csv/daily_temperature_extracted.csv`

File này chứa dữ liệu nhiệt độ theo ngày, ví dụ:

- `Date`
- `Daily_Min_C`
- `Daily_Max_C`

Nó phù hợp cho phân tích phụ trợ hoặc mở rộng feature, nhưng không phải input chính của luồng inference hiện tại.

## File sinh ra và artefact

Không nên nhầm các file build / model đã xuất với source chính.

### Trong firmware STM32

Các thư mục / file sau thường là artefact build của Keil/MDK:

- `Objects/`
- `Listings/`
- `DebugConfig/`
- `RTE/`
- `.axf`
- `.htm`
- `.lst`
- `.crf`
- `.d`
- `.dep`
- `.sct`

### Trong ML

Các file sau là artefact hoặc file deploy:

- `soil_model_dt.pkl`
- `mlp_model.pkl`
- `scaler.pkl`
- `mlp_model.c`
- `mlp_model.h`

## Cách đọc repo nhanh nhất

Nếu muốn nắm flow trong thời gian ngắn, nên đọc theo thứ tự:

1. [source_code/soil_moisture_dht11_stm32/main.c](source_code/soil_moisture_dht11_stm32/main.c)
2. [source_code/gateway_esp32/src/main.cpp](source_code/gateway_esp32/src/main.cpp)
3. [train_AI/MLP_TRAIN/train_mlp.py](train_AI/MLP_TRAIN/train_mlp.py)
4. [train_AI/Decision_Tree/train.py](train_AI/Decision_Tree/train.py)
5. [source_csv/Smart_Farming_Smart_Labeled.csv](source_csv/Smart_Farming_Smart_Labeled.csv)

## Tóm tắt ngắn

`sensor_nw_final` không phải một project đơn lẻ mà là chuỗi đầy đủ gồm:

- phần cứng và tài liệu thiết kế
- firmware STM32 đọc cảm biến
- gateway ESP32 xử lý AI và MQTT
- dataset CSV
- pipeline train Decision Tree và MLP

Nếu đi theo đúng flow trên, người đọc sẽ hiểu được dữ liệu đi từ cảm biến nào, qua firmware nào, model nào xử lý, và telemetry cuối cùng được đẩy đi đâu.

Điểm mạnh của cấu trúc hiện tại là đã có đủ các thành phần để triển khai end-to-end. Điểm cần chỉnh trước khi chạy lại là **đồng bộ tên cột dữ liệu**, **phân biệt source với artefact**, và **xác nhận đúng thư mục chạy script**.
