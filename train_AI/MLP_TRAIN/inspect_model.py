import joblib
import numpy as np

# =====================================================
# LOAD FILE
# =====================================================

model = joblib.load("mlp_model.pkl")
scaler = joblib.load("scaler.pkl")

# =====================================================
# MODEL INFO
# =====================================================

print("========== MODEL INFO ==========")

print("Model type:")
print(type(model))

print("\nActivation:")
print(model.activation)

print("\nSolver:")
print(model.solver)

print("\nHidden layers:")
print(model.hidden_layer_sizes)

print("\nNumber of layers:")
print(model.n_layers_)

print("\nOutput neurons:")
print(model.n_outputs_)

# =====================================================
# WEIGHTS
# =====================================================

print("\n========== WEIGHTS ==========")

for i, w in enumerate(model.coefs_):
    print(f"\nLayer {i} weights shape:")
    print(w.shape)

    print("Weights:")
    print(w)

# =====================================================
# BIAS
# =====================================================

print("\n========== BIAS ==========")

for i, b in enumerate(model.intercepts_):
    print(f"\nLayer {i} bias shape:")
    print(b.shape)

    print("Bias:")
    print(b)

# =====================================================
# SCALER INFO
# =====================================================

print("\n========== SCALER ==========")

print("\nMean:")
print(scaler.mean_)

print("\nStandard deviation:")
print(scaler.scale_)