import paho.mqtt.client as mqtt
import signal

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

mi = client.publish('manager/blimps/b01/config', "k_p_z=0.774 k_d_z=0.774 k_i_z=0.5 k_p_xy=0.774 k_d_xy=0.774 k_p_a=0.774 k_d_a=0.774", 0, False) 

mi.wait_for_publish()
client.disconnect()

print("Exit")
