import joblib
from sklearn.tree import export_text

# =====================================================
# 1. LOAD MODEL TỪ FILE .PKL
# =====================================================
model = joblib.load("soil_model_dt.pkl")
print("Đã load model từ soil_model_dt.pkl")

# =====================================================
# 2. IN RULE CÂY QUYẾT ĐỊNH
# =====================================================
feature_names = ["soil_moisture", "temperature", "humidity"]
rules = export_text(model, feature_names=feature_names)

print("\n===== DECISION TREE RULES =====")
print(rules)