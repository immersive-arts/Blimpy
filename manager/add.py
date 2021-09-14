import paho.mqtt.client as mqtt
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("mqtt_broker_ip", help="IP address of mqtt broker")
parser.add_argument("mqtt_broker_port", help="port number of MQTT broker")
parser.add_argument("manager_base_topic", help="base topic of manager")
parser.add_argument("device_base_topic", help="base topic of device")
parser.add_argument("device_type", help="type of device to be added")
parser.add_argument("device_name", help="name of device to be added")
parser.add_argument("tracking_id", help="tracking id of device to be added")

args = parser.parse_args()
if args.mqtt_broker_ip:
    BROKER_IP = str(args.mqtt_broker_ip)
if args.mqtt_broker_port:
    BROKER_PORT = int(args.mqtt_broker_port)
if args.manager_base_topic:
    MANAGER_BASE_TOPIC = str(args.manager_base_topic)
if args.device_base_topic:
    DEVICE_BASE_TOPIC = str(args.device_base_topic)
if args.device_type:
    DEVICE_TYPE = str(args.device_type)
if args.device_name:
    DEVICE_NAME = str(args.device_name)
if args.tracking_id:
    TRACKING_ID = str(args.tracking_id)

client = mqtt.Client()
client.username_pw_set(username="testtest",password="testtest")
client.connect(BROKER_IP, BROKER_PORT, 60)
print("Client connected to %s on port %d" % (BROKER_IP, BROKER_PORT))
client.loop_start()

command = "device_base_topic=%s device_type=%s device_name=%s tracking_id=%s" % (DEVICE_BASE_TOPIC, DEVICE_TYPE, DEVICE_NAME, TRACKING_ID)
mi = client.publish(MANAGER_BASE_TOPIC + '/add', command, 0, False)
mi.wait_for_publish()

print("Published %s to '%s/add'" % (command, MANAGER_BASE_TOPIC))
client.disconnect()

print("Exit")
