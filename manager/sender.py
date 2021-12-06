import paho.mqtt.client as mqtt
import time
import signal
import numpy as np

def signal_handler(sig, frame):
    global run
    run = False
    
signal.signal(signal.SIGINT, signal_handler)

class Sender:
    def __init__(self, BROKER_IP, BROKER_PORT, MANAGER_BASE_TOPIC, DEVICE_BASE_TOPIC, DEVICE_NAME):

        self.topic_base = MANAGER_BASE_TOPIC + '/' + DEVICE_BASE_TOPIC + '/' + DEVICE_NAME + '/'
        self.counter = 0
        self.client = mqtt.Client()
        self.client.username_pw_set(username="testtest",password="testtest")
        self.client.connect(BROKER_IP, BROKER_PORT, 60)
        print("Client connected to %s on port %d" % (BROKER_IP, BROKER_PORT))
        self.client.loop_start()
        
    def move(self):
        global R, freq, z_ref
        t = time.time() - t0
        x_ref = R * np.cos(2 * np.pi * freq * t)
        y_ref = R * np.sin(2 * np.pi * freq * t)
        dx_ref = -R * 2 * np.pi * freq * np.sin(2 * np.pi * freq * t)
        dy_ref = R * 2 * np.pi * freq * np.cos(2 * np.pi * freq * t)
        a_ref = np.arctan2(dy_ref, dx_ref)
        da_ref = 2*np.pi*freq

        command = 'clear; move x=%f y=%f z=%f alpha=%f vx=%f vy=%f valpha=%f' % (x_ref, y_ref, z_ref, a_ref, dx_ref, dy_ref, da_ref)
        #command = 'clear; move x=%f y=%f z=%f alpha=%f vx=%f vy=%f valpha=%f' % (0.0, 0.0, z_ref, a_ref, 0.0, 0.0, 0.0)

        self.counter = self.counter + 1

        self.client.publish(self.topic_base + 'stack', command, 0, False)
        
    def park(self, x, y, z, alpha, t, vmax=0):
        if vmax > 0.0:
            command = 'park x=%f y=%f z=%f alpha=%f t=%f vmax=%f' % (x, y, z, alpha, t, vmax)
        else:
            command = 'park x=%f y=%f z=%f alpha=%f t=%f' % (x, y, z, alpha, t)

        self.counter = self.counter + 1
        self.client.publish(self.topic_base + 'stack', command, 0, False)

    def hack(self):
        global R, freq, z_ref, t_switch, up
        t = time.time() - t0
        x_ref = R * np.cos(2 * np.pi * freq * t)
        y_ref = R * np.sin(2 * np.pi * freq * t)

        if t - t_switch > 20.0:
            if up == 1.0:
                up = 0.0
            elif up == 0.0:
                up = 1.0

            t_switch = t

        command = 'clear; hack x=%f y=%f z=%f up=%f' % (x_ref, y_ref, z_ref, up)

        self.counter = self.counter + 1

        self.client.publish(self.topic_base + 'stack', command, 0, False)

    def clear(self):
        self.client.publish(self.topic_base + 'clear', payload=None, qos=0, retain=False)
        
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("mqtt_broker_ip", help="IP address of mqtt broker")
parser.add_argument("mqtt_broker_port", help="port number of MQTT broker")
parser.add_argument("manager_base_topic", help="base topic of manager")
parser.add_argument("device_base_topic", help="base topic of device")
parser.add_argument("device_name", help="name of device to be added")

args = parser.parse_args()
if args.mqtt_broker_ip:
    BROKER_IP = str(args.mqtt_broker_ip)
if args.mqtt_broker_port:
    BROKER_PORT = int(args.mqtt_broker_port)
if args.manager_base_topic:
    MANAGER_BASE_TOPIC = str(args.manager_base_topic)
if args.device_base_topic:
    DEVICE_BASE_TOPIC = str(args.device_base_topic)
if args.device_name:
    DEVICE_NAME = str(args.device_name)


count = 0
t0 = time.time()
dt = 0.1
R = 2.5
freq = 1/60
z_ref = 2.5
x_ref = R
y_ref = 0.0
t_switch = 0.0
up = 1

run = True
sender = Sender(BROKER_IP, BROKER_PORT, MANAGER_BASE_TOPIC, DEVICE_BASE_TOPIC, DEVICE_NAME)
sender.clear()
sender.park(x_ref, y_ref, z_ref, np.pi/2, 15.0, 0.0)
time.sleep(20.0)

t0 = time.time()
while run:
    print("Run sender")
    sender.hack()
    count = count + 1
    time.sleep(max(0.0, count * dt - (time.time() - t0)))

sender.clear()
time.sleep(0.1)

print("Exit")
