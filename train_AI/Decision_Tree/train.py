import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier, export_text
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import joblib

# =====================================================
# 1. ĐỌC FILE CSV
# =====================================================
df = pd.read_csv("soil_data.csv")

# =====================================================
# 2. KIỂM TRA DỮ LIỆU
# =====================================================
print("===== 5 DÒNG ĐẦU =====")
print(df.head())

print("\n===== THÔNG TIN DATA =====")
print(df.info())

print("\n===== KIỂM TRA NULL =====")
print(df.isnull().sum())

# =====================================================
# 3. INPUT / OUTPUT
# =====================================================
X = df[['soil_moisture', 'temperature', 'humidity']]
y = df['label']

# =====================================================
# 4. CHIA TRAIN / TEST
# =====================================================
X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.2,
    random_state=42,
    stratify=y
)

print("\nTrain size:", len(X_train))
print("Test size :", len(X_test))

# =====================================================
# 5. TẠO MODEL DECISION TREE
# =====================================================
model = DecisionTreeClassifier(
    max_depth=5,
    min_samples_leaf=5,
    random_state=42
)

# =====================================================
# 6. TRAIN MODEL
# =====================================================
model.fit(X_train, y_train)

print("\n===== TRAIN XONG =====")

# =====================================================
# 7. DỰ ĐOÁN
# =====================================================
y_pred = model.predict(X_test)

# =====================================================
# 8. ĐÁNH GIÁ MODEL
# =====================================================
acc = accuracy_score(y_test, y_pred)

print("\n===== KẾT QUẢ =====")
print("Accuracy:", round(acc * 100, 2), "%")

print("\n===== CLASSIFICATION REPORT =====")
print(classification_report(y_test, y_pred))

print("\n===== CONFUSION MATRIX =====")
print(confusion_matrix(y_test, y_pred))

# =====================================================
# 9. XEM CÂY QUYẾT ĐỊNH
# =====================================================
print("\n===== DECISION TREE RULES =====")
rules = export_text(model, feature_names=list(X.columns))
print(rules)

# =====================================================
# 10. ĐỘ QUAN TRỌNG CỦA FEATURE
# =====================================================
features = X.columns
importance = model.feature_importances_

print("\n===== FEATURE IMPORTANCE =====")
for f, imp in zip(features, importance):
    print(f"{f}: {imp:.4f}")

# =====================================================
# 11. LƯU MODEL
# =====================================================
joblib.dump(model, "soil_model_dt.pkl")
print("\nĐã lưu model: soil_model_dt.pkl")

# =====================================================
# 12. TEST THỬ 1 GIÁ TRỊ MỚI
# =====================================================
sample = pd.DataFrame([[20, 32, 70]], columns=['soil_moisture', 'temperature', 'humidity'])
prediction = model.predict(sample)

print("\n===== TEST MẪU MỚI =====")
print("Input:")
print(sample)

if prediction[0] == 1:
    print("Kết quả: CẦN TƯỚI")
else:
    print("Kết quả: KHÔNG CẦN TƯỚI")