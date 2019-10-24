import numpy as np

# motor alignment (use cos(a) for 0/90)
thrust = np.sin(np.pi * 45 / 180)
print("thrust", thrust)

# meter
lever = 1.5
print("lever", lever)

# motor forces and torques matrix (fx, fz, mx, mz)
MATRIX = np.array([
    [thrust, thrust, thrust, thrust],
    [-thrust, thrust, thrust, -thrust],
    [thrust*lever, thrust*-lever, thrust*lever, thrust*-lever],
    [thrust*lever, thrust*lever, thrust*-lever, thrust*-lever]
])
print("MATRIX", MATRIX)

# max motor forces in X
FX_MAX = np.max(np.dot(MATRIX, np.array([1, 1, 1, 1])))
print("FX_MAX", FX_MAX)

# max motor forces in Z
FZ_MAX = np.max(np.dot(MATRIX, np.array([-1, 1, 1, -1])))
print("FZ_MAX", FZ_MAX)

# max motor torques in X
MX_MAX = np.max(np.dot(MATRIX, np.array([1, 1, -1, -1])))
print("MX_MAX", MX_MAX)

# max motor torques in Z
MZ_MAX = np.max(np.dot(MATRIX, np.array([1, 1, -1, -1])))
print("MZ_MAX", MZ_MAX)

# pseudo inverse of matrix
INVERSE = np.linalg.pinv(MATRIX)
print("INVERSE", INVERSE)

# example calculation
f = np.array([0, FZ_MAX/2, 0, 0])
print("example", np.dot(INVERSE, f))
