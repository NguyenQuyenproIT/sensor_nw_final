# SENSOR_NW_FINAL

Tài liệu này mô tả tổng quan hiện trạng của thư mục `sensor_nw_final`, với mục tiêu giúp người đọc nhanh chóng hiểu:

- Dự án này đang gồm những khối nào.
- Dữ liệu đi vào từ đâu, model được train ở đâu, firmware nằm ở đâu.
- Những file nào là nguồn chính, file nào là file sinh tự động, file nào chỉ nên xem như artefact.
- Các điểm cần lưu ý trước khi chạy lại train, build firmware, hoặc mở rộng hệ thống.

Đây là một repository mang tính hệ sinh thái, không phải một project đơn lẻ. Nói cách khác, nó kết hợp **thiết kế phần cứng**, **thu thập dữ liệu**, **huấn luyện ML**, và **firmware nhúng STM32** cho bài toán smart farming / giám sát độ ẩm đất và môi trường.

---

## 1. Bức tranh tổng thể

Luồng chính của repository có thể hiểu như sau:

```mermaid
flowchart LR
    A[Sensor / dữ liệu thực địa] --> B[source_csv/\nCSV dữ liệu]
    B --> C[train_AI/Decision_Tree\ntrain.py]
    B --> D[train_AI/MLP_TRAIN\ntrain_mlp.py]
    C --> E[soil_model_dt.pkl\nDecision Tree model]
    D --> F[mlp_model.pkl\nMLP model]
    D --> G[scaler.pkl\nStandardScaler]
    E --> H[Ứng dụng suy luận / phân loại]
    F --> H
    G --> H
    H --> I[source_code/\nFirmware STM32]
    I --> J[UART / cảm biến / hiển thị dữ liệu]
    K[circuit_design/] --> I
    L[documents/] --> I
```

Ý nghĩa của sơ đồ:

- `source_csv/` là lớp dữ liệu đầu vào.
- `train_AI/` là lớp huấn luyện và xuất model.
- `source_code/` là lớp firmware nhúng trên STM32.
- `circuit_design/` và `documents/` là lớp hỗ trợ cho phần cứng và tài liệu kỹ thuật.

---

## 2. Cấu trúc thư mục

```text
sensor_nw_final/
├── circuit_design/
├── documents/
├── source_code/
├── source_csv/
└── train_AI/
```

### 2.1 `circuit_design/`

Thư mục này chứa các tài nguyên thiết kế mạch / thư viện phần cứng:

- `DEVKIT_V1_ESP32-WROOM-32.IntLib`
- `Stm32 Blue Pill.IntLib`

Đây là các file thư viện tích hợp cho công cụ thiết kế mạch, phục vụ phần phần cứng của hệ thống. Về mặt tài liệu, nên xem đây là **tài nguyên thiết kế**, không phải code chạy trực tiếp.

### 2.2 `documents/`

Thư mục tài liệu, hiện có các nhánh:

- `esp/`
- `mqtt/`

Theo cấu trúc hiện tại, đây là nơi phù hợp để lưu:

- ghi chú giao tiếp ESP32
- ghi chú MQTT / truyền thông
- hướng dẫn wiring / triển khai phần cứng
- các tài liệu mô tả hệ thống

### 2.3 `source_code/`

Đây là phần firmware STM32. Hiện có 2 project độc lập:

- `soil_moisture_stm32/`
- `soil_moisture_dht11_stm32/`

Mỗi project có cấu trúc kiểu Keil/MDK khá đầy đủ, gồm:

- file project `.uvprojx`, `.uvoptx`, `.uvguix.admin`
- thư mục `Objects/`
- thư mục `Listings/`
- `DebugConfig/`
- `RTE/`

Nói ngắn gọn:

- project `soil_moisture_stm32` tập trung vào cảm biến độ ẩm đất qua ADC
- project `soil_moisture_dht11_stm32` kết hợp DHT11 và ADC đất

### 2.4 `source_csv/`

Đây là kho dữ liệu CSV của dự án. Các file đang thấy:

- `daily_temperature_extracted.csv`
- `log_temp.csv`
- `Smart_Farming_Crop_Yield_2024.csv`
- `Smart_Farming_Final_Refined.csv`
- `Smart_Farming_Smart_Labeled.csv`

Các file này cho thấy repo không chỉ có dữ liệu cảm biến thô, mà còn có cả dữ liệu đã xử lý và dữ liệu đã gán nhãn để huấn luyện ML.

### 2.5 `train_AI/`

Đây là phần huấn luyện mô hình.

Hai nhánh chính hiện có:

- `Decision_Tree/`
- `MLP_TRAIN/`

Mỗi nhánh có script train, file dữ liệu, và file model sinh ra sau training.

---

## 3. Phân tích chi tiết theo nhánh

## 3.1 `source_code/soil_moisture_stm32/`

File trung tâm: [source_code/soil_moisture_stm32/main.c](source_code/soil_moisture_stm32/main.c)

### Chức năng chính

Project này đọc giá trị ADC từ chân analog, tính mức độ ẩm đất theo công thức tỷ lệ nghịch, rồi in kết quả ra UART1.

### Các khối chức năng nổi bật

- Cấu hình `USART1` ở baud rate `115200` để xuất log.
- Cấu hình `TIM2` làm bộ tạo trễ vi mô và mili giây.
- Cấu hình `ADC1` trên `PA0` để đọc tín hiệu analog từ cảm biến độ ẩm đất.
- Lấy trung bình 10 mẫu ADC để giảm nhiễu.
- Chuyển giá trị ADC thành phần trăm độ ẩm theo công thức:

```text
moisture = (4095 - adc_value) * 100 / 4095
```

### Output quan sát được

Chương trình in ra theo dạng:

```text
ADC = <value> | Moisture = <percent>%
```

### Ý nghĩa kỹ thuật

- Đây là firmware tối giản, phục vụ đọc cảm biến và kiểm thử phần cứng.
- Dữ liệu xuất ra UART có thể dùng để debug, logging, hoặc làm nguồn dữ liệu cho tầng khác.

## 3.2 `source_code/soil_moisture_dht11_stm32/`

File trung tâm: [source_code/soil_moisture_dht11_stm32/main.c](source_code/soil_moisture_dht11_stm32/main.c)

### Chức năng chính

Project này mở rộng hơn, gồm 2 nhóm cảm biến:

- DHT11 trên `PB12`
- ADC đọc độ ẩm đất trên `PA0`

### Các khối chức năng nổi bật

- `UART1_Config()` để in log serial.
- `TIM2_Config()` để tạo delay chính xác.
- `DHT11_GPIO_Config()` để kéo chân DHT11 về input pull-up.
- `DHT11_Start()`, `DHT11_Read_Byte()`, `DHT11_Read_Data()` để giao tiếp với DHT11 ở mức bit-banging.
- `ADC_Config()`, `Read_ADC()`, `Read_ADC_Average()` để đọc độ ẩm đất tương tự project còn lại.

### Ý nghĩa kỹ thuật

- Đây là bản firmware có ý đồ kết hợp nhiều nguồn dữ liệu cảm biến.
- Nó phù hợp cho hệ thống mà logic điều khiển cần cả nhiệt độ, độ ẩm không khí, và độ ẩm đất.

### Điểm cần chú ý

- Hàm đọc DHT11 có timeout ngắn, phù hợp cho thực nghiệm nhưng cần kiểm tra thực tế nếu dây dài hoặc tín hiệu nhiễu.
- Cấu hình GPIO chuyển qua lại giữa open-drain, input floating, và input pull-up là phần nhạy cảm nhất của project này.

---

## 4. Phân tích phần ML

## 4.1 `train_AI/Decision_Tree/`

File chính: [train_AI/Decision_Tree/train.py](train_AI/Decision_Tree/train.py)

### Mục đích

Huấn luyện mô hình cây quyết định để phân loại tình trạng tưới nước dựa trên 3 đầu vào:

- `soil_moisture`
- `temperature`
- `humidity`

### Pipeline thực hiện

1. Đọc `soil_data.csv`.
2. Kiểm tra dữ liệu đầu vào.
3. Tách feature và label.
4. Chia train/test theo tỉ lệ 80/20.
5. Train `DecisionTreeClassifier`.
6. Đánh giá bằng accuracy, classification report, confusion matrix.
7. In luật cây bằng `export_text()`.
8. Lưu model ra `soil_model_dt.pkl`.

### Kết quả đầu ra

- Model `.pkl`
- Báo cáo đánh giá trên console
- Luật cây quyết định để giải thích mô hình

### Giá trị thực tế

- Đây là model dễ giải thích nhất trong repo.
- Phù hợp nếu cần rule-based interpretation để kiểm tra logic tưới.

## 4.2 `train_AI/MLP_TRAIN/`

File chính: [train_AI/MLP_TRAIN/train_mlp.py](train_AI/MLP_TRAIN/train_mlp.py)

File hỗ trợ: [train_AI/MLP_TRAIN/read_model.py](train_AI/MLP_TRAIN/read_model.py)

### Mục đích

Huấn luyện mô hình neural network nhiều lớp để phân loại bài toán tương tự decision tree, nhưng theo hướng học phi tuyến mạnh hơn.

### Pipeline thực hiện

1. Đọc `soil_data.csv`.
2. Xáo trộn dữ liệu.
3. Tách `X` và `y`.
4. Chia train/test.
5. Chuẩn hóa feature bằng `StandardScaler`.
6. Train `MLPClassifier`.
7. Đánh giá model.
8. Lưu model và scaler.

### Artefact sinh ra

- `mlp_model.pkl`
- `scaler.pkl`

### Vai trò của `read_model.py`

Script này dùng để:

- nạp lại model đã train
- xem kiến trúc hidden layers
- in weights / biases
- xem tham số scaler

Nó rất hữu ích khi cần kiểm tra model trước khi đem sang môi trường triển khai.

### Điểm kỹ thuật quan trọng

- MLP bắt buộc đi kèm scaler khi suy luận.
- Thiếu `scaler.pkl` thì pipeline inference sẽ không khớp với pipeline training.

---

## 5. Dữ liệu đầu vào

### `source_csv/Smart_Farming_Smart_Labeled.csv`

File mẫu đang cho thấy các cột:

- `soil_moisture`
- `temperature`
- `humidity`
- `lable`

Đây là chi tiết rất quan trọng.

Trong các script train hiện tại, code đang truy cập cột `label`, nhưng dữ liệu mẫu hiển thị cột là `lable`. Nếu đây là tên cột thật trong file đang dùng, thì script sẽ lỗi khi chạy. Cần thống nhất tên cột trước khi train.

### `source_csv/daily_temperature_extracted.csv`

File này có cấu trúc thời gian theo ngày, ví dụ:

- `Date`
- `Daily_Min_C`
- `Daily_Max_C`

Điều này cho thấy repository có cả lớp dữ liệu khí hậu / nhiệt độ ngày, có thể dùng cho phân tích hoặc làm feature phụ trợ.

---

## 6. Những file sinh tự động và artefact cần phân biệt rõ

Đây là phần rất quan trọng để tránh nhầm lẫn giữa source và output.

### 6.1 Trong firmware STM32

Các file / thư mục sau thường là artefact của Keil/MDK hoặc quá trình build:

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

Những file này không nên xem là code gốc để chỉnh logic hệ thống.

### 6.2 Trong ML

Các file model đã lưu cũng là artefact:

- `soil_model_dt.pkl`
- `mlp_model.pkl`
- `scaler.pkl`
- `mlp_model.c`
- `mlp_model.h`

Một số file trong nhóm này phục vụ deployment / export, không phải nguồn train gốc.

---

## 7. Vấn đề cần lưu ý trước khi chạy lại project

### 7.1 Vấn đề tên cột dữ liệu

Đây là điểm cần kiểm tra đầu tiên:

- file CSV mẫu đang có cột `lable`
- script train đang đọc cột `label`

Nếu không sửa thống nhất, training sẽ lỗi ngay ở bước đọc dữ liệu hoặc tách target.

### 7.2 Vấn đề đường dẫn tương đối

Các script Python đang đọc `soil_data.csv` bằng đường dẫn tương đối. Điều đó có nghĩa là:

- bạn cần chạy script từ đúng thư mục chứa file CSV
- hoặc cần chỉnh path trong script nếu chạy từ root

### 7.3 Vấn đề toolchain STM32

Phần firmware đang đi theo cấu trúc Keil/MDK và SPL của STM32F1. Muốn build lại cần đúng môi trường, đúng chip target, và đúng thư viện đi kèm.

### 7.4 Vấn đề đồng bộ giữa firmware và model

Nếu hệ thống cuối cùng dùng model để ra quyết định tưới, thì cần đảm bảo:

- thứ tự input feature giống nhau
- đơn vị đo giống nhau
- cách tiền xử lý giống nhau
- mapping nhãn output giống nhau

Chỉ cần lệch một bước là kết quả thực thi sẽ sai dù model train vẫn “đẹp”.

---

## 8. Gợi ý cách hiểu repo theo từng vai trò

Bạn có thể coi repository này theo 4 vai trò chính:

1. **Thu thập dữ liệu**: từ cảm biến, log, hoặc dataset CSV.
2. **Huấn luyện mô hình**: Decision Tree và MLP.
3. **Triển khai nhúng**: firmware STM32 đọc cảm biến và xuất dữ liệu.
4. **Tài liệu / phần cứng**: sơ đồ, thư viện mạch, ghi chú giao tiếp.

Đây là kiến trúc hợp lý cho một hệ thống nông nghiệp thông minh thử nghiệm, vì nó cho phép tách riêng phần đo đạc, phần phân tích, và phần điều khiển.

---

## 9. Tài liệu / file quan trọng nhất để bắt đầu đọc

Nếu bạn muốn hiểu nhanh nhất repo này, hãy đọc theo thứ tự sau:

1. [source_code/soil_moisture_stm32/main.c](source_code/soil_moisture_stm32/main.c)
2. [source_code/soil_moisture_dht11_stm32/main.c](source_code/soil_moisture_dht11_stm32/main.c)
3. [train_AI/Decision_Tree/train.py](train_AI/Decision_Tree/train.py)
4. [train_AI/MLP_TRAIN/train_mlp.py](train_AI/MLP_TRAIN/train_mlp.py)
5. [source_csv/Smart_Farming_Smart_Labeled.csv](source_csv/Smart_Farming_Smart_Labeled.csv)

---

## 10. Kết luận ngắn

`sensor_nw_final` là một repo kết hợp đầy đủ các lớp của một hệ thống smart farming:

- phần cứng cảm biến
- firmware STM32
- dữ liệu CSV
- mô hình machine learning
- tài liệu và thiết kế mạch

Điểm mạnh của cấu trúc hiện tại là đã có đủ các thành phần để triển khai end-to-end. Điểm cần chỉnh trước khi chạy lại là **đồng bộ tên cột dữ liệu**, **phân biệt source với artefact**, và **xác nhận đúng thư mục chạy script**.
