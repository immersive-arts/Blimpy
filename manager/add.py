import paho.mqtt.client as mqtt
import signal

def signal_handler(sig, frame):
    global run
    run = False
    
signal.signal(signal.SIGINT, signal_handler)
        
host = "10.128.96.204"
port = 1883
client = mqtt.Client()
client.username_pw_set(username="testtest",password="testtest")
client.connect(host, port, 60)
print("Client connected to %s on port %d" % (host, port))
client.loop_start()

mi = client.publish('manager/add', "blimp_base_topic=blimps blimp_id=b01 tracking_id=1", 0, False) 

mi.wait_for_publish()
client.disconnect()

print("Exit")
