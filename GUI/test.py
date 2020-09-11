import socket
import time
import numpy as np
import signal as sig
import sys
import math
import struct

def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')
    sys.exit(0)
    
sig.signal(sig.SIGINT, signal_handler)


MCAST_GRP = '224.1.1.1'
MCAST_PORT = 7001
MULTICAST_TTL = 2
t0 = time.time()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)

t0 = time.time()
dt = 0.1
while True:
    x = np.cos(2*np.pi*1/60*(time.time()-t0))
    y = np.sin(2*np.pi*1/60*(time.time()-t0))
    z = 2.0
    alpha = 0.0
    vx = 0.0
    vy = 0.0
    vz = 0.0
    valpha = 0.0
    x_ref = np.cos(2*np.pi*1/60*(time.time()-t0))
    y_ref = np.sin(2*np.pi*1/60*(time.time()-t0))
    z_ref = 2.0
    alpha_ref = 0.0
    vx_ref = 0.0
    vy_ref = 0.0
    vz_ref = 0.0
    valpha_ref = 0.0
    fx = 1.0
    fy = 2.0
    fz = 3.0
    malpha = 1.1
    m1 = 0.1
    m2 = 0.2
    m3 = 0.3
    m4 = 0.4
    m5 = 0.5
    m6 = 0.6
    
    sample = np.array([x, y, z, alpha, vx, vy, vz, valpha, x_ref, y_ref, z_ref, alpha_ref, vx_ref, vy_ref, vz_ref, valpha_ref, fx, fy, fz, malpha, m1, m2, m3, m4, m5, m6])
    data = struct.pack('<26f',*sample)
    sock.sendto(data, (MCAST_GRP, MCAST_PORT))
    
    t = time.time() - t0
    print(t)
    count = math.ceil(t / dt)
    time.sleep(max(0, dt*count - t))