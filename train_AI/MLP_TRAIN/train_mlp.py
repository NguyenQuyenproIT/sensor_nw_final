import pandas as pd
import joblib

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPClassifier

from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix
)

# =====================================================
# 1. READ DATA
# =====================================================

df = pd.read_csv("soil_data_ver3.csv")

# SHUFFLE DATA
df = df.sample(frac=1, random_state=42).reset_index(drop=True)

print(df.head())

# =====================================================
# 2. INPUT / OUTPUT
# =====================================================

X = df[['soil_moisture', 'temperature', 'humidity']]
y = df['label']

# =====================================================
# 3. SPLIT DATA
# =====================================================

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.2,
    stratify=y,
    random_state=42
)

# =====================================================
# 4. NORMALIZE
# =====================================================

scaler = StandardScaler()

X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# =====================================================
# 5. CREATE MODEL
# =====================================================

model = MLPClassifier(
    hidden_layer_sizes=(16, 8),
    activation='tanh',
    solver='adam',
    learning_rate_init=0.001,
    max_iter=5000,
    early_stopping=True,
    random_state=42
)

# =====================================================
# 6. TRAIN
# =====================================================

print("\nTRAINING...")

model.fit(X_train, y_train)

print("DONE!")

# =====================================================
# 7. PREDICT
# =====================================================

y_pred = model.predict(X_test)

# =====================================================
# 8. EVALUATE
# =====================================================

print("\nAccuracy:", accuracy_score(y_test, y_pred))

print("\nClassification Report:")
print(classification_report(y_test, y_pred))

print("\nConfusion Matrix:")
print(confusion_matrix(y_test, y_pred))

# =====================================================
# 9. SAVE
# =====================================================

joblib.dump(model, "mlp_model.pkl")
joblib.dump(scaler, "scaler.pkl")

print("\nMODEL SAVED")