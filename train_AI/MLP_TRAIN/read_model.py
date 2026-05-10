import joblib

# LOAD MODEL
model = joblib.load("mlp_model.pkl")

# LOAD SCALER
scaler = joblib.load("scaler.pkl")

# =========================
# MODEL INFO
# =========================

print("Hidden layers:")
print(model.hidden_layer_sizes)

print("\nActivation:")
print(model.activation)

# =========================
# WEIGHTS
# =========================

for i, w in enumerate(model.coefs_):
    print(f"\nLayer {i} weights:")
    print(w)

# =========================
# BIAS
# =========================

for i, b in enumerate(model.intercepts_):
    print(f"\nLayer {i} bias:")
    print(b)

# =========================
# SCALER
# =========================

print("\nScaler mean:")
print(scaler.mean_)

print("\nScaler std:")
print(scaler.scale_)