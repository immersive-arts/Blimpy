import numpy as np

# motor alignment (use cos(a) for 0/90)
thrust = np.sin(np.pi * 45 / 180)

# meter
lever = 0.5

# motor forces and torques (fx, fz, mz)
A = np.array([
    [thrust, thrust, thrust, thrust],
    # translation in y is not possible
    [-thrust, thrust, thrust, -thrust],
    [thrust*lever, thrust*lever, thrust*-lever, thrust*-lever], # mz
])

# max duty cycle
u_max = 255

# max motor forces in X
m = np.array([1, 1, 1, 1]) * u_max
FX_MAX = np.max(np.dot(A, m))
print("FX_MAX", FX_MAX)

# max motor forces in Z
m = np.array([-1, 1, 1, -1]) * u_max
FZ_MAX = np.max(np.dot(A, m))
print("FZ_MAX",FZ_MAX)

# max motor torques in Z
m = np.array([1, 1, -1, -1]) * u_max
MZ_MAX = np.max(np.dot(A, m))
print("MZ_MAX", MZ_MAX)

# pseudo inverse of matrix
B = np.linalg.pinv(A)
print(B)

# example

FX = FX_MAX
FZ = 0
MZ = MZ_MAX

f = np.array([FX, FZ, MZ])
m = np.dot(B, f)
print(m)
