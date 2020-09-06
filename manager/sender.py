import paho.mqtt.client as mqtt
import time
import signal
import numpy as np

def signal_handler(sig, frame):
    global run
    run = False
    
signal.signal(signal.SIGINT, signal_handler)
        
class Sender:
    def __init__(self):
        host = "localhost"
        port = 1883
        self.counter = 0
        self.client = mqtt.Client()
        self.client.username_pw_set(username="testtest",password="testtest")
        self.client.connect(host, port, 60)
        print("Client connected to %s on port %d" % (host, port))
        self.client.loop_start()
        
    def send(self):
        command = 'clear'      
        self.client.publish("manager", command, 0, False) 
        
    def stack(self):
        global R, freq, z_ref
        t = time.time() - t0
        x_ref = R * np.cos(2 * np.pi * freq * t)
        y_ref = R * np.sin(2 * np.pi * freq * t)
        dx_ref = - R * 2 * np.pi * freq * np.sin(2 * np.pi * freq * t)
        dy_ref = R * 2 * np.pi * freq * np.cos(2 * np.pi * freq * t)
        a_ref = np.arctan2(dy_ref, dx_ref)

        command = 'move x=%f y=%f z=%f vx=%f vy=%f alpha=%f' % (x_ref, y_ref, z_ref, dx_ref, dy_ref, a_ref)
        self.counter = self.counter + 1
        self.client.publish('manager/blimps/b01/stack', command, 0, False)
        
    def stack_hold(self):
        global R, freq, z_ref
        t = time.time() - t0
        x_ref = R * np.cos(2 * np.pi * freq * t)
        y_ref = R * np.sin(2 * np.pi * freq * t)
        dx_ref = - R * 2 * np.pi * freq * np.sin(2 * np.pi * freq * t)
        dy_ref = R * 2 * np.pi * freq * np.cos(2 * np.pi * freq * t)
        a_ref = np.arctan2(dy_ref, dx_ref)

        command = 'hold x=%f y=%f z=%f vx=%f vy=%f alpha=%f' % (x_ref, y_ref, z_ref, dx_ref, dy_ref, a_ref)
        self.counter = self.counter + 1
        self.client.publish('manager/blimps/b01/stack', command, 0, False)
        
    def stack_goto(self):
        global R, freq, z_ref
        t = time.time() - t0
        x_ref = 1.0
        y_ref = 1.0
        z_ref = 2.0
        a_ref = 0.0
        t = 20.0

        command = 'goto x=%f y=%f z=%f alpha=%f t=%f' % (x_ref, y_ref, z_ref, a_ref, t)
        self.counter = self.counter + 1
        self.client.publish('manager/blimps/b01/stack', command, 0, False)
    
    def clear(self):
        self.client.publish('manager/blimps/b01/clear', payload=None, qos=0, retain=False)

count = 0
t0 = time.time()
dt = 0.1
R = 2.0
freq = 1/60
z_ref = 2.0

run = True
sender = Sender()
while run:
    print("Run sender")
    sender.clear()
    sender.stack()
    count = count + 1
    time.sleep(max(0.0, count * dt - (time.time() - t0)))
    
sender.clear()
sender.stack_hold()
time.sleep(3.0)
sender.clear()
sender.stack_goto()
time.sleep(0.1)

print("Exit")
