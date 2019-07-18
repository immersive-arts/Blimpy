import numpy as np

# motor alignment (use cos(a) for 0/90)
thrust = np.sin(np.pi * 45 / 180)
print(thrust)

# motor forces
A = np.array([
    [thrust, thrust, thrust, thrust], # x
    # [0, 0, 0, 0], # y
    [-thrust, thrust, thrust, -thrust], # z
])

# max unit of force (duty cycle)
m_max = 255

# max motor forces in X
m = np.array([1, 1, 1, 1]) * m_max
FX_MAX = np.dot(A, m)
print(FX_MAX)

# max motor forces in Z
m = np.array([-1, 1, 1, -1]) * m_max
FZ_MAX = np.dot(A, m)
print(FZ_MAX)

# pseudo inverse of matrix
B = np.linalg.pinv(A)
print(B)

# example

FX = FX_MAX[0]
FZ = 0

f = np.array([FX, FZ])

m = np.dot(B, f)
print(m)
