import joblib
import pandas as pd

# =====================================================
# 1. LOAD MODEL TỪ FILE .PKL
# =====================================================
model = joblib.load("soil_model_dt.pkl")
print("Đã load model từ soil_model_dt.pkl")

# =====================================================
# 2. TEST 1 MẪU MỚI
# =====================================================
sample = pd.DataFrame([[20, 32, 70]], columns=["soil_moisture", "temperature", "humidity"])

print("\n===== SAMPLE TEST =====")
print(sample)

# =====================================================
# 3. DỰ ĐOÁN
# =====================================================
prediction = model.predict(sample)[0]

print("\n===== KẾT QUẢ =====")
if prediction == 1:
    print("CẦN TƯỚI")
else:
    print("KHÔNG CẦN TƯỚI")

# =====================================================
# 4. XEM XÁC SUẤT (NẾU MODEL HỖ TRỢ)
# =====================================================
if hasattr(model, "predict_proba"):
    proba = model.predict_proba(sample)[0]
    print("\n===== XÁC SUẤT =====")
    print(f"Class 0 (không tưới): {proba[0]:.4f}")
    print(f"Class 1 (cần tưới)   : {proba[1]:.4f}")