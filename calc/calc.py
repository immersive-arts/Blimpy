import numpy as np

# motor alignment (use cos(a) for 0/90)
thrust = np.sin(np.pi * 45 / 180)
print("thrust", thrust)

# meter
leverX = 1.5
leverY = 2.5
print("levers", leverX, leverY)

# motor forces and torques matrix (fx, fy, fz, mx, mz)
MATRIX = np.array([
    [thrust, thrust, thrust, thrust, 0, 0],
    [0, 0, 0, 0, 1, 1],
    [-thrust, thrust, thrust, -thrust, 0, 0],
    [thrust * leverX, thrust * -leverX, thrust * leverX, thrust * -leverX, thrust * -leverY, thrust * leverY],
    [thrust * leverX, thrust * leverX, thrust * -leverX, thrust * -leverX, 0, 0]
])
print("MATRIX", MATRIX)

# max motor forces in X
FX_MAX = np.max(np.dot(MATRIX, np.array([1, 1, 1, 1, 0, 0])))
print("FX_MAX", FX_MAX)

# max motor forces in Y
FY_MAX = np.max(np.dot(MATRIX, np.array([0, 0, 0, 0, 1, 1])))
print("FY_MAX", FY_MAX)

# max motor forces in Z
FZ_MAX = np.max(np.dot(MATRIX, np.array([-1, 1, 1, -1, 0, 0])))
print("FZ_MAX", FZ_MAX)

# max motor torques in X
MX_MAX = np.max(np.dot(MATRIX, np.array([1, -1, 1, -1, -1, 1])))
print("MX_MAX", MX_MAX)

# max motor torques in Z
MZ_MAX = np.max(np.dot(MATRIX, np.array([1, 1, -1, -1, 0, 0])))
print("MZ_MAX", MZ_MAX)

# pseudo inverse of matrix
INVERSE = np.linalg.pinv(MATRIX)
print("INVERSE", INVERSE)

# example calculation
f = np.array([0, FZ_MAX/2, 0, 0, 0])
print("example", np.dot(INVERSE, f))
