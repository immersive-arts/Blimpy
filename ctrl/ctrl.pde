import mqtt.*;

MQTTClient client;

void setup() {
  size(500, 500);
  frameRate(5);
  
  client = new MQTTClient(this);
  client.connect("mqtt://testtest:testtest@broker.shiftr.io", "processing");
}

void draw() {
  background(0);
}

void keyPressed() {
  switch(keyCode) {
    case 38: // up
    {
      setMotors(0.0, 1.0, 0.0, 1.0);
      break;
    }
    case 37: // left
    {
      setMotors(1.0, 1.0, 0.0, 0.0);
      break;
    }
    case 39: // right
    {
      setMotors(0.0, 0.0, 1.0, 1.0);
      break;
    }
    case 40: // down
    {
      setMotors(1.0, 0.0, 1.0, 0.0);
      break;
    }
  }
}

void keyReleased() {
  setMotors(0.0, 0.0, 0.0, 0.0); 
}

void setMotors(float m1, float m2, float m3, float m4) {
  client.publish("blimpy/m1", Float.toString(m1 * 255));
  client.publish("blimpy/m2", Float.toString(m2 * 255));
  client.publish("blimpy/m3", Float.toString(m3 * 255));
  client.publish("blimpy/m4", Float.toString(m4 * 255));
}

void clientConnected() {
  println("connected");

  client.subscribe("/hello");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
}

void connectionLost() {
  println("disconnected");
}
