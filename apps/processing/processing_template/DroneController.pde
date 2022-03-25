import mqtt.*;

/**
    DroneController handles MQTT requests
    Use serviceUrl "mqtt://public:public@public.cloud.shiftr.io" to test your code
 */
public class DroneController implements MQTTListener
{
    // Member Variables
    private MQTTClient client;
    private String serviceUrl;
    private String name;
    private String deviceId;
    private float battery;
    private float[] motors = new float[]{0,0,0,0,0,0};
    private float[] servos = new float[]{0,0,0,0,0,0};

    // Getter
    public float GetBattery() 
    {
        return battery;
    }
    public float[] getMotors() 
    {
        return motors;
    }
    public float[] getServos() 
    {
        return servos;
    }

    /**
        Constructor
        @param processingRef reference to the processing PApplet (this)
        @param serviceUrl url of the broker service (incl. port)
        @param deviceId name of your device (blimps/<NAME>)
        @param name name of your endpoint node
     */ 
    public DroneController(PApplet processingRef, String serviceUrl, String deviceId, String name) 
    {
        // init variables
        this.serviceUrl = serviceUrl;
        this.name = name;
        this.deviceId = deviceId;

        // Create MQTT client and connect to it
        client = new MQTTClient(processingRef, this);
        client.connect(serviceUrl, name);
    }
    public DroneController(PApplet processingRef, String serviceUrl, String deviceId) 
    {
        this(processingRef, serviceUrl, deviceId, "processing");
    }

    /**
        Automatically subscribe to the device by its id
     */
    void clientConnected() 
    {
        println("client connected");
        client.subscribe(deviceId);
        client.subscribe(deviceId + "/battery");
    }

    /**
        This method is called once the client received a message.
        @param topic The topic.
        @param payload The payload.
     */
    void messageReceived(String topic, byte[] payload) 
    {
        // Payload to String
        String s = new String(payload);

        if (topic.endsWith("battery")) 
        {
            // save Battery Life
            battery = Float.parseFloat(s.split(",")[0]);
        }
        else 
        {
            // log unhandled Request
            println("new message: " + topic + " - " + new String(payload));
        }
    }

    /**
        This method is called once the client has lost connection.
     */
    void connectionLost() 
    {
        println("connection lost");
    }
    /**
        Send Input to the Device (from cache)
        @param use motors/servos as topic
     */
    private void SendInput(String topic) 
    {
        // Get state from array 
        float[] values;
        switch (topic) {
            case "motors": values = motors; break;	
            case "servos": values = servos; break;	
            default: return;
        }
        // Send string
        String valueString = join(nf(values, 0, 6), ",");
        client.publish(deviceId + "/" + topic, valueString);
        println("sent " + topic + " directions " + valueString);
    }

    /**
        Send Input to the Device
        @param use motors/servos as topic
        @param values array of 6 floats wich determine the speed of the motors / the angle of the servos
     */
    public void SendInput(String topic, float[] values) 
    {
        switch (topic) {
            case "motors": motors = values; break;	
            case "servos": servos = values; break;	
            default: return;
        }
        SendInput(topic);
    }

    /**
        Send Input to Motors
        @param values array of 6 floats wich determine the speed of the motors
     */
    public void SendMotorInput(float val1, float val2, float val3, float val4, float val5, float val6) 
    {
        motors = new float[]{ val1, val2, val3, val4, val5, val6 };
        SendInput("motors");
    }

    /**
        Send Input to Motors
        @param index from 0 to 5 wich motor can be accessed
        @param val float from  0 to 1 wich determines the speed of the motor
     */
    public void SendMotorInput(int index, float val) 
    {
        if (index >= 0 && index <= 5) 
        {
            motors[index] = val;
            SendInput("motors");
        }
    }

    /**
        Send Input to Servo
        @param values array of 6 floats wich determine the angle of the servos
     */
    public void SendServoInput(float val1, float val2, float val3, float val4, float val5, float val6) 
    {
        servos = new float[]{ val1, val2, val3, val4, val5, val6 };
        SendInput("servos");
    }

    /**
        Send Input to Servo
        @param index from 0 to 5 wich servo can be accessed
        @param val float from  0 to 1 wich determines the angle of the servo
     */
    public void SendServoInput(int index, float val) 
    {
        if (index >= 0 && index <= 5) 
        {
            servos[index] = val;
            SendInput("servos");
        }
    }
}
