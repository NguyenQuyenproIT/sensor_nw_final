import pandas as pd
import numpy as np

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score
)

# =====================================================
# 1. CONFIG
# =====================================================
CSV_FILES = [
    "soil_data.csv",
    "soil_data_ver3.csv",
]

FEATURE_COLS = ["soil_moisture", "temperature", "humidity"]
TARGET_COL = "label"

RANDOM_STATE = 42
TEST_SIZE = 0.2

# Model config
MODEL_PARAMS = {
    "hidden_layer_sizes": (16, 8),
    "activation": "tanh",
    "solver": "adam",
    "learning_rate_init": 0.001,
    "alpha": 0.0001,
    "batch_size": "auto",
    "max_iter": 5000,
    "early_stopping": True,
    "validation_fraction": 0.15,
    "n_iter_no_change": 30,
    "random_state": RANDOM_STATE,
    "verbose": False
}

# =====================================================
# 2. CLEAN FUNCTION
# =====================================================
def load_and_clean(csv_path):
    print("\n" + "=" * 70)
    print(f"LOADING: {csv_path}")
    print("=" * 70)

    df = pd.read_csv(csv_path)

    print("\nRaw shape:", df.shape)
    print("Columns:", df.columns.tolist())

    required_cols = FEATURE_COLS + [TARGET_COL]
    missing_cols = [c for c in required_cols if c not in df.columns]
    if missing_cols:
        raise ValueError(f"{csv_path} missing columns: {missing_cols}")

    print("\nNulls before clean:")
    print(df[required_cols].isnull().sum())

    # Convert to numeric
    for col in required_cols:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    # Fill feature NaN with median
    for col in FEATURE_COLS:
        df[col] = df[col].fillna(df[col].median())

    # Drop missing labels
    df = df.dropna(subset=[TARGET_COL])

    # Drop duplicates
    dup_count = df.duplicated().sum()
    print("Duplicates before drop:", dup_count)
    df = df.drop_duplicates().reset_index(drop=True)

    # Cast label to int
    df[TARGET_COL] = df[TARGET_COL].astype(int)

    print("\nNulls after clean:")
    print(df[required_cols].isnull().sum())

    print("\nShape after clean:", df.shape)
    print("\nClass distribution:")
    print(df[TARGET_COL].value_counts())

    return df

# =====================================================
# 3. TRAIN + EVALUATE
# =====================================================
def train_and_evaluate(df, name):
    print("\n" + "#" * 70)
    print(f"TRAINING ON: {name}")
    print("#" * 70)

    # Shuffle
    df = df.sample(frac=1, random_state=RANDOM_STATE).reset_index(drop=True)

    X = df[FEATURE_COLS]
    y = df[TARGET_COL]

    X_train, X_test, y_train, y_test = train_test_split(
        X, y,
        test_size=TEST_SIZE,
        stratify=y,
        random_state=RANDOM_STATE
    )

    print("\nSplit info:")
    print("Train size:", len(X_train))
    print("Test size :", len(X_test))

    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)

    model = MLPClassifier(**MODEL_PARAMS)

    print("\nTraining...")
    model.fit(X_train_scaled, y_train)
    print("Done.")

    y_pred = model.predict(X_test_scaled)

    acc = accuracy_score(y_test, y_pred)
    f1_macro = f1_score(y_test, y_pred, average="macro")

    print("\n===== RESULTS =====")
    print(f"Accuracy : {acc * 100:.2f}%")
    print(f"F1 macro : {f1_macro:.4f}")
    print(f"Iterations: {model.n_iter_}")
    print(f"Loss     : {model.loss_:.6f}")

    print("\n===== CLASSIFICATION REPORT =====")
    print(classification_report(y_test, y_pred, digits=4))

    print("\n===== CONFUSION MATRIX =====")
    print(confusion_matrix(y_test, y_pred))

    return {
        "name": name,
        "accuracy": acc,
        "f1_macro": f1_macro,
        "model": model,
        "scaler": scaler,
        "y_test": y_test,
        "y_pred": y_pred
    }

# =====================================================
# 4. MAIN
# =====================================================
results = []

for csv_path in CSV_FILES:
    df = load_and_clean(csv_path)
    result = train_and_evaluate(df, csv_path)
    results.append(result)

# =====================================================
# 5. COMPARE RESULTS
# =====================================================
print("\n" + "=" * 70)
print("FINAL COMPARISON")
print("=" * 70)

for r in results:
    print(
        f"{r['name']} -> "
        f"Accuracy: {r['accuracy'] * 100:.2f}% | "
        f"F1 macro: {r['f1_macro']:.4f}"
    )

best_by_acc = max(results, key=lambda x: x["accuracy"])
best_by_f1 = max(results, key=lambda x: x["f1_macro"])

print("\nBest by Accuracy:", best_by_acc["name"], f"({best_by_acc['accuracy'] * 100:.2f}%)")
print("Best by F1 macro:", best_by_f1["name"], f"({best_by_f1['f1_macro']:.4f})")