import numpy as np

# motor alignment (use cos(a) for 0/90)
thrust = np.sin(np.pi * 45 / 180)

# meter
lever = 0.5

# motor forces and torques matrix (fx, fz, mz)
MATRIX = np.array([
    [thrust, thrust, thrust, thrust],
    [-thrust, thrust, thrust, -thrust],
    [thrust*lever, thrust*lever, thrust*-lever, thrust*-lever]
])

# max duty cycle
max_duty_cycle = 255

# max motor forces in X
FX_MAX = np.max(np.dot(MATRIX, np.array([1, 1, 1, 1]) * max_duty_cycle))
print("FX_MAX", FX_MAX)

# max motor forces in Z
FZ_MAX = np.max(np.dot(MATRIX, np.array([-1, 1, 1, -1]) * max_duty_cycle))
print("FZ_MAX", FZ_MAX)

# max motor torques in Z
MZ_MAX = np.max(np.dot(MATRIX, np.array([1, 1, -1, -1]) * max_duty_cycle))
print("MZ_MAX", MZ_MAX)

# pseudo inverse of matrix
INVERSE = np.linalg.pinv(MATRIX)
print("INVERSE", INVERSE)

# example calculation
f = np.array([FX_MAX, 0, MZ_MAX])
print("example", np.dot(INVERSE, f))
