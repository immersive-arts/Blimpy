import paho.mqtt.client as mqtt
import time
import signal
import numpy as np

def signal_handler(sig, frame):
    global run
    run = False
    
signal.signal(signal.SIGINT, signal_handler)
        
host = "localhost"
port = 1883
client = mqtt.Client()
client.username_pw_set(username="testtest",password="testtest")
client.connect(host, port, 60)
print("Client connected to %s on port %d" % (host, port))
client.loop_start()

mi = client.publish('manager/remove', "blimp_id=b01", 0, False)

mi.wait_for_publish()
client.disconnect()

print("Exit")
