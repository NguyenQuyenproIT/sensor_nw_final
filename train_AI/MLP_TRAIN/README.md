MLP_TRAIN pipeline - README

Mục đích: huấn luyện MLP, kiểm tra, và xuất trọng số để nhúng vào firmware.

Files:

- `train_mlp.py` — script train (pandas, scikit-learn). Configure data path and model hyperparameters here.
- `read_model.py` — nạp model + scaler, in thông tin.
- `inspect_model.py` — inspect weights/bias để xuất sang C.
- `requirements.txt` — (nên có) liệt kê pandas scikit-learn joblib numpy.

Flow:

1. Chuẩn bị CSV (repair header 'lable' → 'label' nếu cần).
2. Chạy `train_mlp.py` → sinh `mlp_model.pkl`, `scaler.pkl`.
3. Dùng `inspect_model.py` để xuất ma trận và vector.

Gợi ý export:

- Viết script tự động chuyển `mlp_model.pkl` + `scaler.pkl` → `mlp_model.c` & `mlp_model.h`.# MLP_TRAIN - Hệ Thống Phân Loại Đất Bằng Mạng Neural MLP

> **Phần này hướng dẫn huấn luyện mô hình Multilayer Perceptron (MLP) để phân loại các loại đất dựa trên các cảm biến môi trường**

---

## 📋 Nội Dung

1. [Giới Thiệu Lý Thuyết](#giới-thiệu-lý-thuyết)
2. [Cấu Trúc Project](#cấu-trúc-project)
3. [Mô Tả Chi Tiết Các File](#mô-tả-chi-tiết-các-file)
4. [Kiến Trúc Mô Hình](#kiến-trúc-mô-hình)
5. [Quy Trình Huấn Luyện](#quy-trình-huấn-luyện)
6. [Hướng Dẫn Sử Dụng](#hướng-dẫn-sử-dụng)
7. [Giải Thích Chi Tiết Các Bước](#giải-thích-chi-tiết-các-bước)
8. [Kết Quả & Đánh Giá](#kết-quả--đánh-giá)

---

## 🧠 Giới Thiệu Lý Thuyết

### Multilayer Perceptron (MLP) Là Gì?

**MLP** (hay còn gọi là Feedforward Neural Network) là một loại mạng neural nhân tạo được sử dụng rộng rãi để:
- ✅ **Phân loại** (Classification) - Chia dữ liệu thành các nhóm khác nhau
- ✅ **Dự đoán** (Regression) - Dự đoán giá trị liên tục
- ✅ **Nhận dạng mẫu** (Pattern Recognition)

### Các Lợi Ích Của MLP

| Lợi Ích | Mô Tả |
|---------|-------|
| **Khả năng học phi tuyến** | Có thể học các mối quan hệ phức tạp giữa đầu vào và đầu ra |
| **Linh hoạt** | Có thể giải quyết nhiều bài toán khác nhau |
| **Hiệu suất cao** | Cho kết quả tốt khi được huấn luyện đúng cách |
| **Dễ triển khai** | Có nhiều library hỗ trợ (scikit-learn, TensorFlow, v.v.) |

### Cách Hoạt Động Của MLP

MLP hoạt động theo nguyên tắc:

1. **Đầu vào (Input Layer)**: Nhận các feature từ dữ liệu
2. **Các lớp ẩn (Hidden Layers)**: Xử lý và trích xuất đặc trưng từ dữ liệu
3. **Đầu ra (Output Layer)**: Đưa ra dự đoán cuối cùng
4. **Hàm kích hoạt (Activation Function)**: Thêm tính phi tuyến để mô hình học được các mẫu phức tạp

### Công Thức Toán Học

Trong mỗi neuron, tính toán diễn ra như sau:

$$z = W \cdot x + b$$

Sau đó áp dụng hàm kích hoạt:

$$a = activation(z)$$

Trong đó:
- $W$ = trọng số (weights)
- $x$ = đầu vào
- $b$ = độ lệch (bias)
- $a$ = đầu ra

---

## 📁 Cấu Trúc Project

```
MLP_TRAIN/
│
├── train_mlp.py           ⭐ File chính - Huấn luyện mô hình
├── inspect_model.py       🔍 Chi tiết mô hình - Xem chi tiết weights, bias, scaler
├── read_model.py          📖 Đọc mô hình - Hiển thị thông tin cơ bản
│
├── soil_data.csv          📊 Dữ liệu huấn luyện
│
├── mlp_model.pkl          💾 Model đã huấn luyện (được tạo sau khi chạy train_mlp.py)
├── scaler.pkl             💾 Scaler đã lưu (được tạo sau khi chạy train_mlp.py)
│
└── README.md              📚 Hướng dẫn này
```

---

## 📄 Mô Tả Chi Tiết Các File

### ⭐ **1. train_mlp.py** - FILE CHÍNH

**Mục đích**: Huấn luyện mô hình MLP từ đầu

**Quy trình**:
1. ✅ Đọc dữ liệu từ `soil_data.csv`
2. ✅ Xáo trộn dữ liệu (shuffle) để tránh bias
3. ✅ Tách đầu vào (X) và đầu ra (y)
4. ✅ Chia dữ liệu thành tập huấn luyện (80%) và tập kiểm tra (20%)
5. ✅ Chuẩn hóa dữ liệu bằng StandardScaler
6. ✅ Tạo mô hình MLP với cấu hình tối ưu
7. ✅ Huấn luyện mô hình trên dữ liệu
8. ✅ Dự đoán trên tập kiểm tra
9. ✅ Đánh giá hiệu suất (Accuracy, Precision, Recall, F1-score)
10. ✅ Lưu mô hình và scaler để sử dụng sau

**Các tham số quan trọng**:
```python
MLPClassifier(
    hidden_layer_sizes=(16, 8),    # 2 lớp ẩn: 16 và 8 neuron
    activation='tanh',              # Hàm kích hoạt: Tangent Hyperbolic
    solver='adam',                  # Thuật toán tối ưu: Adam
    learning_rate_init=0.001,       # Tốc độ học ban đầu
    max_iter=5000,                  # Số vòng lặp tối đa
    early_stopping=True,            # Dừng sớm nếu không cải thiện
    random_state=42                 # Cố định ngẫu nhiên để tái tạo kết quả
)
```

---

### 🔍 **2. inspect_model.py** - CHI TIẾT MÔ HÌNH

**Mục đích**: Kiểm tra chi tiết các thành phần bên trong mô hình

**Thông tin hiển thị**:
- 📊 Loại hình: MLPClassifier
- 🎯 Hàm kích hoạt: tanh
- ⚙️ Thuật toán tối ưu: adam
- 🏗️ Kích thước các lớp ẩn: (16, 8)
- 🔢 Số lượng lớp: 4 (1 input + 2 hidden + 1 output)
- 📤 Số neuron đầu ra: 2 (các nhóm đất)
- ⚖️ **Weights** (Trọng số) của từng lớp:
  - Layer 0: 3×16 (từ 3 input đến 16 hidden neurons)
  - Layer 1: 16×8 (từ 16 sang 8 hidden neurons)
  - Layer 2: 8×2 (từ 8 sang 2 output neurons)
- 📍 **Bias** (Độ lệch) của từng lớp
- 📏 **Scaler parameters**: Mean và Standard deviation cho chuẩn hóa dữ liệu

**Khi nào sử dụng**: Khi bạn muốn hiểu rõ các trọng số và độ lệch của mô hình

---

### 📖 **3. read_model.py** - ĐỌC MÔ HÌNH

**Mục đích**: Hiển thị thông tin cơ bản về mô hình đã huấn luyện

**Thông tin hiển thị**:
- Cấu trúc lớp ẩn
- Hàm kích hoạt sử dụng
- Các trọng số (weights) của mỗi lớp
- Các độ lệch (bias) của mỗi lớp
- Thông tin về scaler

**Khi nào sử dụng**: Kiểm tra nhanh thông tin mô hình mà không cần chi tiết quá

---

### 📊 **4. soil_data.csv** - DỮ LIỆU

**Cấu trúc dữ liệu**:
```
soil_moisture,temperature,humidity,label
45.2,28.5,65.3,Group_A
52.1,26.8,72.5,Group_B
...
```

**Các cột (Features)**:
| Cột | Mô Tả | Phạm Vi |
|-----|-------|---------|
| `soil_moisture` | Độ ẩm của đất (%) | 0-100 |
| `temperature` | Nhiệt độ môi trường (°C) | -10 đến +50 |
| `humidity` | Độ ẩm không khí (%) | 0-100 |
| `label` | Loại đất (Group_A, Group_B) | - |

**Nhận xét**: 
- Dữ liệu được xáo trộn ngẫu nhiên trước khi sử dụng
- Dữ liệu được chuẩn hóa trước khi đưa vào mô hình

---

## 🏗️ Kiến Trúc Mô Hình

### Sơ Đồ Kiến Trúc MLP

```
┌─────────────────────────────────────────────────────────────┐
│                  INPUT LAYER (3 neurons)                    │
│              ┌─────────────────────────────┐                │
│              │ soil_moisture               │                │
│              │ temperature                 │                │
│              │ humidity                    │                │
│              └─────────────────────────────┘                │
└──────────────────────────┬──────────────────────────────────┘
                           │ (3 features)
                           ▼
┌─────────────────────────────────────────────────────────────┐
│            HIDDEN LAYER 1 (16 neurons)                      │
│    activation: tanh                                         │
│    [N1][N2][N3][N4][N5][N6][N7][N8]                         │
│    [N9][N10][N11][N12][N13][N14][N15][N16]                  │
└──────────────────────────┬──────────────────────────────────┘
                           │ (16 features)
                           ▼
┌─────────────────────────────────────────────────────────────┐
│            HIDDEN LAYER 2 (8 neurons)                       │
│    activation: tanh                                         │
│    [N1][N2][N3][N4][N5][N6][N7][N8]                         │
└──────────────────────────┬──────────────────────────────────┘
                           │ (8 features)
                           ▼
┌─────────────────────────────────────────────────────────────┐
│             OUTPUT LAYER (2 neurons)                        │
│    activation: softmax (cho classification)                 │
│           [Group_A]   [Group_B]                             │
│    Xác suất: P(A)     P(B)                                  │
└─────────────────────────────────────────────────────────────┘
```

### Tổng Quan Kiến Trúc

```
Input Features (3)
       │
       ├─→ W1 (3×16) + b1 ──→ tanh ──→ Hidden Layer 1 (16)
       │
       ├─→ W2 (16×8) + b2 ──→ tanh ──→ Hidden Layer 2 (8)
       │
       ├─→ W3 (8×2) + b3 ──→ softmax ──→ Output (2)
       │
       └─→ Predictions (Group_A hoặc Group_B)
```

---

## 🔄 Quy Trình Huấn Luyện

### Sơ Đồ Luồng Dữ Liệu

```
┌──────────────────────┐
│  soil_data.csv       │  📊 Dữ liệu gốc (1000+ mẫu)
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│  SHUFFLE DATA        │  🔀 Xáo trộn để tránh bias
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│  SPLIT (80/20)       │  ✂️ Tách thành:
│  - Train: 80%        │     • Tập train: 80% (huấn luyện)
│  - Test: 20%         │     • Tập test: 20% (kiểm tra)
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│  NORMALIZE DATA      │  📏 Chuẩn hóa giá trị:
│  StandardScaler      │     • Công thức: (x - mean) / std
└──────────┬───────────┘
           │
    ┌──────┴──────┐
    │             │
    ▼             ▼
┌────────────┐ ┌────────────┐
│X_train norm│ │X_test norm │
└────────┬───┘ └────────┬───┘
         │              │
         │   ┌──────────┘
         │   │
         ▼   ▼
      ┌──────────────┐
      │  MLP Model   │
      │  (16,8)      │  🧠 Huấn luyện mô hình
      └──────┬───────┘
             │
             ▼
      ┌──────────────┐
      │  y_pred      │  🎯 Dự đoán trên tập test
      └──────┬───────┘
             │
             ├─→ Accuracy Score      ✅
             ├─→ Precision/Recall    📊
             ├─→ Confusion Matrix    🗂️
             │
             ▼
      ┌──────────────┐
      │  Save Model  │  💾 Lưu để dùng lại
      │ & Scaler     │
      └──────────────┘
```

---

## 🚀 Hướng Dẫn Sử Dụng

### Bước 1: Chuẩn Bị Môi Trường

```bash
# Cài đặt các thư viện cần thiết
pip install pandas scikit-learn joblib numpy
```

### Bước 2: Chuẩn Bị Dữ Liệu

Đảm bảo file `soil_data.csv` có định dạng:
```csv
soil_moisture,temperature,humidity,label
45.2,28.5,65.3,Group_A
52.1,26.8,72.5,Group_B
...
```

### Bước 3: Huấn Luyện Mô Hình

```bash
python train_mlp.py
```

**Kết quả**:
- ✅ In ra sample dữ liệu
- ✅ Hiển thị quá trình huấn luyện
- ✅ Hiển thị độ chính xác (Accuracy)
- ✅ Hiển thị báo cáo phân loại (Classification Report)
- ✅ Hiển thị ma trận nhầm lẫn (Confusion Matrix)
- ✅ Lưu `mlp_model.pkl` và `scaler.pkl`

### Bước 4: Kiểm Tra Mô Hình

```bash
# Xem thông tin cơ bản
python read_model.py

# Xem chi tiết (weights, bias, scaler)
python inspect_model.py
```

### Bước 5: Sử Dụng Mô Hình Để Dự Đoán

```python
import joblib

# Tải mô hình và scaler
model = joblib.load("mlp_model.pkl")
scaler = joblib.load("scaler.pkl")

# Dữ liệu mới (phải có 3 features)
new_data = [[48.5, 27.2, 68.9]]

# Chuẩn hóa dữ liệu
new_data_normalized = scaler.transform(new_data)

# Dự đoán
prediction = model.predict(new_data_normalized)
print(f"Dự đoán: {prediction[0]}")

# Lấy xác suất
probabilities = model.predict_proba(new_data_normalized)
print(f"Xác suất: {probabilities[0]}")
```

---

## 📚 Giải Thích Chi Tiết Các Bước

### 1️⃣ ĐỌC DỮ LIỆU

```python
df = pd.read_csv("soil_data.csv")
df = df.sample(frac=1, random_state=42).reset_index(drop=True)
```

**Giải thích**:
- `pd.read_csv()`: Đọc file CSV vào DataFrame
- `.sample(frac=1)`: Xáo trộn tất cả các hàng (frac=1 = 100%)
- `random_state=42`: Cố định seed để kết quả lặp lại được
- `.reset_index(drop=True)`: Reset lại index từ 0

**Tại sao lại xáo trộn?** Nếu dữ liệu được sắp xếp (ví dụ: Group_A ở đầu, Group_B ở cuối), mô hình có thể bị bias.

---

### 2️⃣ CHIA INPUT/OUTPUT

```python
X = df[['soil_moisture', 'temperature', 'humidity']]
y = df['label']
```

**Giải thích**:
- `X`: Các đặc trưng đầu vào (features) - Những gì mô hình "nhìn thấy"
- `y`: Nhãn đầu ra (label/target) - Những gì mô hình cần "dự đoán"

**Visualization**:
```
     X (3 features)                    y (target)
┌────────────────────────┐        ┌──────────────┐
│ Soil_M  Temp   Humid   │        │ Label        │
├────────────────────────┤        ├──────────────┤
│ 45.2    28.5   65.3    │ ──────→│ Group_A      │
│ 52.1    26.8   72.5    │        │ Group_B      │
│ 48.5    27.2   68.9    │        │ Group_A      │
│ ...     ...    ...     │        │ ...          │
└────────────────────────┘        └──────────────┘
```

---

### 3️⃣ CHIA TRAIN/TEST

```python
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, stratify=y, random_state=42
)
```

**Giải thích**:
- `test_size=0.2`: Lấy 20% cho kiểm tra, 80% cho huấn luyện
- `stratify=y`: Đảm bảo tỉ lệ các nhóm trong train/test giống nhau
- Ví dụ: Nếu toàn bộ có 60% Group_A, 40% Group_B, thì train/test cũng thế

**Tại sao phải chia?**
- 🎓 Tập train: Mô hình học từ dữ liệu này
- 🧪 Tập test: Kiểm tra xem mô hình có hoạt động tốt trên dữ liệu chưa từng thấy không

**Tỉ lệ phổ biến**: 80/20 hoặc 70/30

```
Toàn bộ dữ liệu (100%)
├─ Train (80%)  ──→ Mô hình học từ đây
└─ Test (20%)   ──→ Kiểm tra khách quan
```

---

### 4️⃣ CHUẨN HÓA DỮ LIỆU

```python
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)
```

**Giải thích**:
- StandardScaler chuẩn hóa các feature về trung bình 0, độ lệch chuẩn 1

**Công thức**:
$$X_{normalized} = \frac{X - mean}{std}$$

**Tại sao chuẩn hóa?**
1. ⚡ Tốc độ học nhanh hơn (converge nhanh)
2. 🎯 Kết quả ổn định hơn
3. 📊 Tránh các feature có giá trị lớn "áp đảo" các feature khác

**Ví dụ**:
```
Dữ liệu gốc:
soil_moisture: 0-100 (phạm vi lớn)
temperature: -10 to +50 (phạm vi nhỏ)
humidity: 0-100 (phạm vi lớn)

→ Không được chuẩn hóa sẽ có các vấn đề.

Dữ liệu chuẩn hóa:
soil_moisture: -2.1 to +2.3 (trung bình 0)
temperature: -1.5 to +1.8 (trung bình 0)
humidity: -1.9 to +2.2 (trung bình 0)

→ Tất cả đều có phạm vi tương đương!
```

---

### 5️⃣ TẠO MÔ HÌNH

```python
model = MLPClassifier(
    hidden_layer_sizes=(16, 8),
    activation='tanh',
    solver='adam',
    learning_rate_init=0.001,
    max_iter=5000,
    early_stopping=True,
    random_state=42
)
```

**Các tham số**:

| Tham số | Giá Trị | Ý Nghĩa |
|---------|---------|---------|
| `hidden_layer_sizes` | (16, 8) | 2 lớp ẩn: 16 neuron ở lớp 1, 8 ở lớp 2 |
| `activation` | 'tanh' | Hàm kích hoạt: Tangent Hyperbolic |
| `solver` | 'adam' | Thuật toán tối ưu hoá: Adam |
| `learning_rate_init` | 0.001 | Tốc độ học ban đầu (bước nhảy cập nhật) |
| `max_iter` | 5000 | Số vòng lặp tối đa |
| `early_stopping` | True | Dừng nếu không cải thiện (tránh overfitting) |
| `random_state` | 42 | Cố định kết quả |

**Hàm kích hoạt (Activation Function)**:
- **tanh**: Giá trị từ -1 đến +1, thường tốt cho hidden layers
- **relu**: Giá trị từ 0 đến vô cực, nhanh nhưng có thể bất ổn
- **logistic**: Giá trị từ 0 đến 1, mượt mà

---

### 6️⃣ HUẤN LUYỆN

```python
model.fit(X_train, y_train)
```

**Quá trình diễn ra**:
1. Mô hình được khởi tạo với trọng số ngẫu nhiên
2. Với mỗi epoch (vòng lặp):
   - ✅ Tính toán dự đoán trên dữ liệu train
   - ✅ Tính lỗi (so sánh với nhãn thực tế)
   - ✅ Cập nhật trọng số để giảm lỗi
3. Quá trình lặp lại cho đến khi convergence hoặc max_iter

**Early Stopping**: Nếu lỗi trên validation set không cải thiện trong N epoch liên tiếp, dừng huấn luyện (tránh overfitting)

---

### 7️⃣ DỰ ĐOÁN

```python
y_pred = model.predict(X_test)
```

**Quá trình**:
- Dữ liệu test được đưa vào mô hình
- Mô hình tính toán qua các lớp
- Lớp output trả về xác suất cho mỗi lớp
- Chọn lớp có xác suất cao nhất

---

### 8️⃣ ĐÁNH GIÁ

```python
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix

accuracy = accuracy_score(y_test, y_pred)
print(classification_report(y_test, y_pred))
print(confusion_matrix(y_test, y_pred))
```

**Các chỉ số**:

| Chỉ Số | Công Thức | Ý Nghĩa |
|--------|-----------|---------|
| **Accuracy** | Đúng / Tổng | Tỉ lệ dự đoán đúng |
| **Precision** | TP / (TP + FP) | Trong những dự đoán "dương tính", bao nhiêu là đúng |
| **Recall** | TP / (TP + FN) | Trong tất cả "dương tính" thực, mô hình bắt được bao nhiêu |
| **F1-score** | 2 × (P × R) / (P + R) | Cân bằng giữa Precision và Recall |

**Confusion Matrix** (Ma Trận Nhầm Lẫn):
```
              Dự đoán: Group_A  Group_B
Thực tế:
Group_A          TP              FN
Group_B          FP              TN

TP (True Positive): Dự đoán đúng Group_A
TN (True Negative): Dự đoán đúng Group_B
FP (False Positive): Dự đoán là Group_A nhưng thực là Group_B
FN (False Negative): Dự đoán là Group_B nhưng thực là Group_A
```

---

### 9️⃣ LƯU MÔ HÌNH

```python
import joblib

joblib.dump(model, "mlp_model.pkl")
joblib.dump(scaler, "scaler.pkl")
```

**Tại sao lại lưu?**
- 💾 Không cần phải huấn luyện lại mô hình mỗi lần
- ⏱️ Huấn luyện mất nhiều thời gian, lưu chỉ mất vài giây khi dùng
- 📦 Dễ dàng chia sẻ mô hình cho người khác

**Các file được tạo**:
- `mlp_model.pkl`: Mô hình đã huấn luyện (trọng số, bias, v.v.)
- `scaler.pkl`: Thông tin chuẩn hóa (mean, std của dữ liệu train)

---

## 📊 Kết Quả & Đánh Giá

### Hiểu Kết Quả Đầu Ra

**Khi chạy `train_mlp.py`, bạn sẽ thấy**:

```
   soil_moisture  temperature  humidity label
0           45.2          28.5      65.3 Group_A
1           52.1          26.8      72.5 Group_B
2           48.5          27.2      68.9 Group_A
3           51.3          26.4      70.8 Group_B
4           46.7          28.9      66.2 Group_A

TRAINING...
DONE!

Accuracy: 0.95

Classification Report:
              precision    recall  f1-score   support

     Group_A       0.94      0.96      0.95       105
     Group_B       0.96      0.94      0.95       103

    accuracy                           0.95       208
   macro avg       0.95      0.95      0.95       208
weighted avg       0.95      0.95      0.95       208

Confusion Matrix:
[[101   4]
 [  6  97]]

MODEL SAVED
```

**Giải thích**:
- ✅ **Accuracy 0.95**: Mô hình dự đoán đúng 95% dữ liệu test
- ✅ **Precision 0.94-0.96**: Khi dự đoán, mô hình có độ chính xác cao
- ✅ **Recall 0.94-0.96**: Mô hình bắt được hầu hết các mẫu đúng
- ✅ **F1-score 0.95**: Cân bằng tốt giữa precision và recall

---

### Các Kỹ Thuật Cải Thiện Hiệu Suất

Nếu độ chính xác không đủ tốt, bạn có thể:

1. **Tăng dữ liệu** 📊
   - Hơn 1000 mẫu sẽ cho kết quả tốt hơn

2. **Điều chỉnh kiến trúc** 🏗️
   - Thử `hidden_layer_sizes=(32, 16)` hoặc `(8, 4)`
   - Thêm hoặc bớt lớp ẩn

3. **Thay đổi hàm kích hoạt** ⚡
   - Thử `activation='relu'` hoặc `'logistic'`

4. **Điều chỉnh learning rate** 🎚️
   - Tăng: `learning_rate_init=0.01` (học nhanh nhưng có thể không hội tụ)
   - Giảm: `learning_rate_init=0.0001` (học chậm nhưng ổn định)

5. **Điều chỉnh max_iter** 🔄
   - Tăng `max_iter` để huấn luyện lâu hơn

6. **Thay solver** ⚙️
   - Thử `solver='sgd'` hoặc `'lbfgs'`

---

## 🔗 Luồng Công Việc Hoàn Chỉnh

```
1. Chuẩn bị → 2. Đọc dữ liệu → 3. Xáo trộn → 4. Chia train/test
                                                       ↓
7. Lưu mô hình ← 6. Đánh giá ← 5. Dự đoán ← Huấn luyện
                                                       ↑
           Kiểm tra → Điều chỉnh tham số (nếu cần)  ↯

Khi hài lòng:
   → Sử dụng mô hình để dự đoán dữ liệu mới
   → Chia sẻ mô hình cho bộ phận khác
   → Triển khai vào ứng dụng thực tế
```

---

## 💡 Lời Khuyên Thực Tiễn

✅ **NÊN LÀM**:
- 📖 Luôn xáo trộn dữ liệu
- 📊 Luôn chuẩn hóa dữ liệu
- 🧪 Luôn chia dữ liệu train/test
- 📈 Kiểm tra loss curve để đánh giá quá trình học
- 💾 Lưu scaler cùng với model
- 📝 Ghi chú lại tham số tốt nhất

❌ **KHÔNG NÊN LÀM**:
- ❌ Sử dụng toàn bộ dữ liệu cho train (không có test set)
- ❌ Quên chuẩn hóa dữ liệu test bằng scaler từ train
- ❌ Sử dụng tôi scaler mới cho tập test
- ❌ Không kiểm tra overfitting (train accuracy cao nhưng test thấp)
- ❌ Vô tư lựa chọn tham số mà không thử nghiệm

---

## Nếu gặp vấn đề:



1. **Lỗi import**: Cài đặt lại thư viện
   ```bash
   pip install --upgrade pandas scikit-learn joblib numpy
   ```

2. **Lỗi file CSV**: Kiểm tra tên cột và đường dẫn
   ```python
   print(df.columns)  # Xem tên cột
   print(df.shape)    # Xem kích thước
   ```

3. **Mô hình không tốt**: Thử các tham số khác
4. **Lỗi scaler**: Đảm bảo scaler từ train được dùng cho test

---

## 📚 Tham Khảo & Học Thêm

- [Neural Networks - Basics](https://towardsdatascience.com/understanding-neural-networks-what-why-and-how-18ec859eadf8)
- [Machine Learning Course](https://www.coursera.org/learn/machine-learning)


