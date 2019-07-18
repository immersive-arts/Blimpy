import mqtt.*;

MQTTClient client;

void setup() {
  size(500, 500);
  frameRate(5);

  client = new MQTTClient(this);
  client.connect("mqtt://0.0.0.0:8883", "ctrl");
}

void draw() {
  background(0);
}

boolean fwd = false;
boolean bwd = false;

boolean left = false;
boolean right = false;
boolean up = false;
boolean down = false;

void keyPressed() {
  setControls(true);
  sendCommands();
}

void keyReleased() {
  setControls(false);
  sendCommands();
}

void sendCommands() {
  if (fwd) {
    if (left) {
      setMotors(1, 1, 0, 0);
    } else if (right) {
      setMotors(0, 0, 1, 1);
    } else if (up) {
      setMotors(1, 0, 1, 0);
    } else if (down) {
      setMotors(0, 1, 0, 1);
    } else {
      setMotors(1, 1, 1, 1);
    }
  } else if (bwd) {
    if (left) {
      setMotors(0, 0, -1, -1);
    } else if (right) {
      setMotors(-1, -1, 0, 0);
    } else if (up) {
      setMotors(0, -1, 0, -1);
    } else if (down) {
      setMotors(-1, 0, -1, 0);
    } else {
      setMotors(-1, -1, -1, -1);
    }
  } else {
    if (left) {
      setMotors(1, 1, 0, 0);
    } else if (right) {
      setMotors(0, 0, 1, 1);
    } else if (up) {
      setMotors(1, 0, 1, 0);
    } else if (down) {
      setMotors(0, 1, 0, 1);
    } else {
      setMotors(0, 0, 0, 0);
    }
  }
}

void setControls(boolean on) {
  switch(keyCode) {
  case 38: // up
    {
      up = on;
      break;
    }
  case 37: // left
    {
      left = on;
      break;
    }
  case 39: // right
    {
      right = on;
      break;
    }
  case 40: // down
    {
      down = on;
      break;
    }
  case 32: // space
    {
      fwd = on;
      break;
    }
  case 9: // tab
    {
      bwd = on;
      break;
    }
  default:
    {
      println(keyCode);
    }
  }
}

float last = 0;

void setMotors(float m1, float m2, float m3, float m4) {
  // calculate hash
  float hash = m1 + m2 * 1025 + m3 * 1024 * 1024 + m4 * 1024 * 1024 * 1024;
  
  // check if update is needed
  if (last == hash) {
    return;
  }

  // prepare values
  String speeds[] = new String[4];
  speeds[0] = Float.toString(m1 * 255);
  speeds[1] = Float.toString(m2 * 255);
  speeds[2] = Float.toString(m3 * 255);
  speeds[3] = Float.toString(m4 * 255);

  // send speeds
  client.publish("blimpy/m1", join(speeds, ","));

  // save hash
  last = hash;
}

void clientConnected() {
  println("connected");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
}

void connectionLost() {
  println("disconnected");
}
