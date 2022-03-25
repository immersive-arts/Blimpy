/* 
    --------------------------------------------------------
    Processing MQTT Blimpy Drone Template
    --------------------------------------------------------
    This file serves as an example.
    You can just use the DroneController with your own logic.
*/

// Constantes
//public final String URL = "mqtt://public:public@public.cloud.shiftr.io"; < Testing without having to connect a device 
public final String URL =  "mqtt://10.128.96.189:1883";
public final String CLIENT_NAME = "CLIENTNAME";
public final String DEVICE_TOPIC = "blimps/DEVICENAME";

// Variables
PImage m_img;
DroneController controller;
Button startButton;
Button stopButton;

// Setup
void setup() {
    // Setup Window
    m_img = loadImage("bg.jpg");
    size(1200,675);

    // Instatioate Drone Controller
    controller = new DroneController(this, URL, DEVICE_TOPIC, CLIENT_NAME);

    // init Buttons
    startButton = new Button("Start", width/2 - 140, height/2 - 45, 240, 90, true);
    stopButton = new Button("Stop", width/2 + 140, height/2 - 45, 240, 90, true);
}

// Draw Window
void draw() {
    // Draw Background image
    image(m_img, 0, 0);

    // Draw Battery Level
    drawBatteryLevel();

    // Draw Buttons
    startButton.draw();
    stopButton.draw();
}

// Draw Battery
void drawBatteryLevel() 
{
    // Draw Battery Background 
    noStroke();
    fill(255,255,255,50);
    rect(width/2 - 150, height - 50, 300, 30, 10);

    // Draw Battery Level
    fill(255,255,255,100);
    rect(width/2 + 150 - 300*controller.GetBattery(), height - 50, 300*controller.GetBattery(), 30, 10);

    // Label
    fill(255,255,255,200);
    textSize(24);
    textAlign(CENTER);
    text((int)(controller.GetBattery()*100) + " %", width/2, height - 65);

    // Log Batterylevel to console
    if (frameCount % 120 == 1) {
        println(controller.GetBattery());
    }
}

// Keypress - Test sending a signal
void keyPressed() 
{
    if (keyCode == 65) 
    {
        controller.SendMotorInput(.2,-.2,0,0,0,0);
    }
    else 
    {
        controller.SendMotorInput(0,0,0,0,0,0);
    }
}

// Mouseclicked - Handle Button "Events"
void mousePressed() 
{
    if(startButton.isMouseOver()) 
    {
        controller.SendMotorInput(.2,0,0,0,0,0);
    }
    else if(stopButton.isMouseOver()) 
    {
        controller.SendMotorInput(0,0,0,0,0,0);
    }
}
