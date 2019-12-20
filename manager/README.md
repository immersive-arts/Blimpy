# Manager

The manager acts as an abstraction layes between user applications and a fleet of blimps. User applications can register a blimp and send motion commands for a registered blimp, which are employed for controling this blimp on a desired path through space. The interaction between user applications and manager, and manager and blimps is based on the MQTT standard; in addition, the manager expects the blimps' motion data, which is neccessary for controling the blimps, from a motion capture system in OSC format.

## Dependencies

The manager is implemented in Python 3.x, which needs to be installed on a computer dedicated to run the manager. The following python libraries need to be installed in addition (depending on the installation further libraries may be required):

osc4py3 for python 3.x:
```bash
pip install osc4py3
```

paho-mqtt for python 3.x:
```bash
pip install paho-mqtt
```
## Running the application
### Prerequisites
The manager requires
* an instance of shiftr.io as a MQTT broker.
* the motion capture system needs to be operational and a NATNET to OSC translator should provide motion capture data in OSC format.

you can find more info (including the apps) inside the network folder.

### Starting the application
#### Linux
```bash
python3 manager.py --mqtt_host <mqtt host IP> --mqtt_port <mqtt host port> --osc_server <osc receiving IP> --osc_port <osc receiving port> --base_topic <mqtt base topic>
```
#### Windows
```bash
python manager.py --mqtt_host <mqtt host IP> --mqtt_port <mqtt host port> --osc_server <osc receiving IP> --osc_port <osc receiving port> --base_topic <mqtt base topic>
```
Assuming the manager runs on the same machine as the MQTT broker and has IP address "10.128.96.102", the manager would be started with the following command.

```bash
python manager.py --mqtt_host "10.128.96.102" --mqtt_port 1883 --osc_server "10.128.96.102" --osc_port 1880 --manager_name "manager"
```
**--mqtt_host** IP address of MQTT broker

**--mqtt_port** listening port of MQTT broker

**--osc_server** is basically this machines IP

**--osc_port** port the manager is listening to OSC messages from NatNetThree2OSC.

**--manager_name** is also the <base_topic> for addressing the manager via MQTT. This allows to run multiple instances on different or the same machine, distinguished by their base topic.

## API

![alt text](../assets/pix/manager/NetworkProtocolDetails.jpg)

To register a new blimp with the manager, a message to the following topic needs to be sent:
```bash
<base_topic>/add
```
The payload of this message configures the blimp as
```bash
"blimp_base_topic=<base topic of the blimp> blimp_id=<blimp id> tracking_id=<tracking id as per the motion capture system>"
```
To send motion commands to a registered blimp, a message to the following topic needs to be sent:
```bash
<base_topic>/<blimp_base_topic>/<blimp_id>/stack
```
The payload of this message holds the desired position, orientation and velocity of the blimp
```bash
"move x=<position x[m]> y=<position y[m]> z=<position z[m]> vx=<velocity x[m/s]> vy=\velocity y[m/s]> vz=<velocity z[m/s]> alpha=<orientation [rad]>"
```
The motion commands are put in a FIFO queue and executed periodally with 10 Hz. In order to clear the queue, a message to the following topic needs to be sent:
```bash
<base_topic>/<blimp_base_topic>/<blimp_id>/clear
```
To deregister a blimp from the manager, message to the following topic needs to be sent:
```bash
<base_topic>/remove
```
The payload of this message describes the desired blimp
```bash
"blimp_id=<blimp id>"
```

## Credits
Max Kriegleder - max.kriegleder@gmail.com
