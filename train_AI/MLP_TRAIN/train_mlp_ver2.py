import pandas as pd
import joblib
import numpy as np

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix
)

# =====================================================
# 1. CONFIG
# =====================================================
CSV_PATH = "soil_data_ver2.csv"
MODEL_PATH = "mlp_model.pkl"
SCALER_PATH = "scaler.pkl"

RANDOM_STATE = 42
TEST_SIZE = 0.2

FEATURE_COLS = ["soil_moisture", "temperature", "humidity"]
TARGET_COL = "label"

# =====================================================
# 2. LOAD DATA
# =====================================================
print("===== LOADING DATA =====")
df = pd.read_csv(CSV_PATH)

print("\n===== RAW DATA INFO =====")
print("Shape:", df.shape)
print(df.head())

print("\n===== COLUMNS =====")
print(df.columns.tolist())

# =====================================================
# 3. VERIFY REQUIRED COLUMNS
# =====================================================
required_cols = FEATURE_COLS + [TARGET_COL]
missing_cols = [col for col in required_cols if col not in df.columns]
if missing_cols:
    raise ValueError(f"Missing required columns in CSV: {missing_cols}")

# =====================================================
# 4. CLEAN DATA
# =====================================================
print("\n===== NULL CHECK BEFORE CLEAN =====")
print(df[required_cols].isnull().sum())

for col in FEATURE_COLS + [TARGET_COL]:
    df[col] = pd.to_numeric(df[col], errors="coerce")

# Fill missing feature values with median
for col in FEATURE_COLS:
    median_val = df[col].median()
    df[col] = df[col].fillna(median_val)

# Drop rows where label is missing
df = df.dropna(subset=[TARGET_COL])

print("\n===== DUPLICATE CHECK =====")
dup_count = df.duplicated().sum()
print("Duplicates before drop:", dup_count)
df = df.drop_duplicates().reset_index(drop=True)

df[TARGET_COL] = df[TARGET_COL].astype(int)

print("\n===== NULL CHECK AFTER CLEAN =====")
print(df[required_cols].isnull().sum())

print("\n===== DATA SHAPE AFTER CLEAN =====")
print("Shape:", df.shape)

print("\n===== CLASS DISTRIBUTION =====")
print(df[TARGET_COL].value_counts())
print("\nClass ratio:")
print(df[TARGET_COL].value_counts(normalize=True).round(4))

# =====================================================
# 5. SHUFFLE DATA
# =====================================================
df = df.sample(frac=1, random_state=RANDOM_STATE).reset_index(drop=True)

# =====================================================
# 6. SPLIT FEATURES / LABEL
# =====================================================
X = df[FEATURE_COLS]
y = df[TARGET_COL]

# =====================================================
# 7. TRAIN / TEST SPLIT
# =====================================================
X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=TEST_SIZE,
    stratify=y,
    random_state=RANDOM_STATE
)

print("\n===== SPLIT INFO =====")
print("Train size:", X_train.shape[0])
print("Test size :", X_test.shape[0])

print("\nTrain class distribution:")
print(y_train.value_counts())

print("\nTest class distribution:")
print(y_test.value_counts())

# =====================================================
# 8. NORMALIZE FEATURES
# =====================================================
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

print("\n===== SCALING DONE =====")
print("Scaler mean:", np.round(scaler.mean_, 4))
print("Scaler var :", np.round(scaler.var_, 4))

# =====================================================
# 9. CREATE MLP MODEL
# =====================================================
model = MLPClassifier(
    hidden_layer_sizes=(16, 8),
    activation="tanh",
    solver="adam",
    learning_rate_init=0.001,
    alpha=0.0001,
    batch_size="auto",
    max_iter=5000,
    early_stopping=True,
    validation_fraction=0.15,
    n_iter_no_change=30,
    random_state=RANDOM_STATE,
    verbose=True
)

print("\n===== MODEL CONFIG =====")
print(model)

# =====================================================
# 10. TRAIN MODEL
# =====================================================
print("\n===== TRAINING START =====")
model.fit(X_train_scaled, y_train)
print("===== TRAINING DONE =====")

# =====================================================
# 11. PREDICT ON TEST SET
# =====================================================
y_pred = model.predict(X_test_scaled)

# =====================================================
# 12. EVALUATION
# =====================================================
acc = accuracy_score(y_test, y_pred)
print("\n===== EVALUATION =====")
print(f"Accuracy: {acc * 100:.2f}%")

print("\n===== CLASSIFICATION REPORT =====")
print(classification_report(y_test, y_pred, digits=4))

print("\n===== CONFUSION MATRIX =====")
print(confusion_matrix(y_test, y_pred))

# =====================================================
# 13. MODEL INSPECTION
# =====================================================
print("\n===== TRAINING RESULT =====")
print("Number of layers:", model.n_layers_)
print("Number of outputs:", model.n_outputs_)
print("Output activation:", model.out_activation_)
print("Loss:", model.loss_)
print("Iterations:", model.n_iter_)

# =====================================================
# 14. TEST SOME SAMPLE INPUTS
# =====================================================
sample_raw = pd.DataFrame([
    [20, 32, 70],
    [35, 28, 55],
    [10, 36, 40],
], columns=FEATURE_COLS)

sample_scaled = scaler.transform(sample_raw)
sample_pred = model.predict(sample_scaled)
sample_proba = model.predict_proba(sample_scaled)

label_map = {
    0: "CLASS 0",
    1: "CLASS 1",
    2: "CLASS 2"
}

print("\n===== SAMPLE TESTS =====")
for i in range(len(sample_raw)):
    print(f"\nSample {i + 1}:")
    print(sample_raw.iloc[i].to_dict())
    print("Prediction:", label_map.get(sample_pred[i], f"UNKNOWN({sample_pred[i]})"))
    print(f"Proba class 0: {sample_proba[i][0]:.4f}")
    print(f"Proba class 1: {sample_proba[i][1]:.4f}")
    print(f"Proba class 2: {sample_proba[i][2]:.4f}")

# =====================================================
# 15. SAVE MODEL AND SCALER
# =====================================================
joblib.dump(model, MODEL_PATH)
joblib.dump(scaler, SCALER_PATH)

print("\n===== SAVED =====")
print(f"Model saved to: {MODEL_PATH}")
print(f"Scaler saved to: {SCALER_PATH}")