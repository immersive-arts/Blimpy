/* 
    --------------------------------------------------------
    Processing MQTT Blimpy Drone Template
    --------------------------------------------------------
    This file serves as an example.
    You can just use the DroneController with your own logic.
*/

// Constantes
//public final String URL = "mqtt://public:public@public.cloud.shiftr.io"; < Testing without having to connect a device 
public final String URL =  "mqtt://195.176.247.64:2883";
public final String NAME = "birdcontroller";
public final String DEVICE = "blimps/birdy";

// Variables
PImage m_img;
DroneController controller;
TailController tailController;
WingController wingController;
Button rightButton;
Button straightButton;
Button leftButton;
Button playButton;
Button speedUpButton;
Button speedDownButton;

// Setup
void setup() {
    // Setup Window
    m_img = loadImage("bg.jpg");
    size(1200,675);

    // Instantiate Controllers
    controller = new DroneController(this, URL, DEVICE, NAME);
    tailController = new TailController(controller, 0.03, 4);
    wingController = new WingController(controller, 0.003, .8, 10);

    // init Buttons
    rightButton = new Button("", width/2 + width/4 + 100, height/2 + 100, 90, 90, true);
    straightButton = new Button("", width/2 + width/4, height/2 + 100, 90, 90, true);
    leftButton = new Button("", width/2 + width/4 - 100, height/2 + 100, 90, 90, true);
    playButton = new Button("", width/2 - width/4, height/2 + 100, 200, 90, true);
    speedDownButton = new Button("", width/2 - width/4 - 100, height/2 + 200, 45, 45, false);
    speedUpButton = new Button("", width/2 - width/4 + 55, height/2 + 200, 45, 45, false);
}

// Draw Window
void draw() 
{
    step();
    drawUI();
}

// Draw UI
void drawUI() 
{
    // Draw Background image
    image(m_img, 0, 0);
    
    if (false && controller.GetBattery() == 0)  // HACK testing without Device
    {
        drawConnectionError();
    }
    else 
    {
        // Draw Battery Level
        drawBatteryLevel();

        // Draw Buttons
        rightButton.draw();
        straightButton.draw();
        leftButton.draw();
        playButton.draw();
        speedUpButton.draw();
        speedDownButton.draw();

        // Draw Button Arrows
        fill(0,0,0,150);
        triangle(rightButton.x - 10, rightButton.y + 25, rightButton.x - 10 + 30, rightButton.y + 25 + 20, rightButton.x - 10, rightButton.y + 25 + 40);
        circle(straightButton.x, straightButton.y + 45, 35);
        triangle(leftButton.x + 10, leftButton.y + 25, leftButton.x + 10 - 30, leftButton.y + 25 + 20, leftButton.x + 10, leftButton.y + 25 + 40);

        // Draw Button Play
        if (wingController.GetFlying()) 
        {
            rect(playButton.x - 20, playButton.y + 25, 40, 40);
        }
        else {
            triangle(playButton.x - 10, playButton.y + 25, playButton.x - 10 + 30, playButton.y + 25 + 20, playButton.x - 10, playButton.y + 25 + 40);
        }

        // Draw Speed
        noStroke();
        fill(255,255,255,50);
        rect(width/2 - width/4 - 50, height/2 + 200, 100, 45, 10);
        fill(255,255,255,200);
        textSize(24);
        textAlign(CENTER);
        text((int)(wingController.GetTargetSpeed()*100)/100.0, width/2 - width/4, height/2 + 230);

        // Draw Buttons Speed
        fill(0,0,0,150);
        triangle(speedUpButton.x + 13, speedUpButton.y + 15 + 15, speedUpButton.x + 13 + 10, speedUpButton.y + 15, speedUpButton.x + 13 + 20, speedUpButton.y + 15 + 15);
        triangle(speedDownButton.x + 13, speedDownButton.y + 17, speedDownButton.x + 13 + 20, speedDownButton.y + 17, speedDownButton.x + 13 + 10, speedDownButton.y + 17 + 15);

        // Draw Tail
        tailController.DrawPreview(width/2 + width/4, height/2 - 100);
        wingController.DrawPreview(width/2 - width/4, height/2 - 100);
    }
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
}

// Draw Connection Error
void drawConnectionError() 
{
    // Box
    noStroke();
    fill(0,0,0,100);
    rect(width/2 - 480, height/2 - 70, 960, 100, 10);

    // Text
    fill(255,255,255);
    textSize(60);
    textAlign(CENTER);
    text((DEVICE + " not connected").toUpperCase(), width/2, height/2);
}

// Step
void step() 
{
    stepKeyDown();
    tailController.Step();
    wingController.Step();
}

// Step Keydown
void stepKeyDown() 
{
    // Move Tail right
    if ((keyPressed && (keyCode == 39 || keyCode == 68)) || (mousePressed && rightButton.isMouseOver()))
    {
        tailController.MoveRight();
        rightButton.SetSelected(true);
    }
    else 
    {
        rightButton.SetSelected(false);
    }

    // Move Tail straight
    if (keyPressed && keyCode == 32)
    {
        tailController.MoveDirection(0);
        straightButton.SetSelected(true);
    }
    else 
    {
        straightButton.SetSelected(false);
    }

    // Move Tail left
    if ((keyPressed && (keyCode == 37 || keyCode == 65)) || (mousePressed && leftButton.isMouseOver()))
    {
        tailController.MoveLeft();
        leftButton.SetSelected(true);
    }
    else 
    {
        leftButton.SetSelected(false);
    }

    // Speed Up
    if ((keyPressed && (keyCode == 38 || keyCode == 87)) || (mousePressed && speedUpButton.isMouseOver()))
    {
        wingController.SpeedUp();
        speedUpButton.SetSelected(true);
    }
    else 
    {
        speedUpButton.SetSelected(false);
    }

    // Speed Down
    if ((keyPressed && (keyCode == 40 || keyCode == 83)) || (mousePressed && speedDownButton.isMouseOver()))
    {
        wingController.SpeedDown();
        speedDownButton.SetSelected(true);
    }
    else 
    {
        speedDownButton.SetSelected(false);
    }
}

// Mouseclicked - Handle Button "Events"
void mousePressed() 
{
    if(playButton.isMouseOver()) 
    {
        wingController.Toggle();
    }
    if (straightButton.isMouseOver()) 
    {
        tailController.MoveDirection(0);
    }
}

// Keypress - Test sending a signal
void keyPressed() 
{
    // Move Tail straight
    if (keyPressed && keyCode == 48)
    {
        tailController.MoveDirection(0);
    }

    // toggle movement straight
    if (keyPressed && keyCode == 32)
    {
        wingController.Toggle();
    }
}