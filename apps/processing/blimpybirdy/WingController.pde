public class WingController
{
    // Member Variables
    DroneController controller;
    float speedIncrease = 0.01;
    float speed = 3;
    float targetSpeed = speed;
    int updateFrequency = 1;
    boolean flying = false;
    float anim = 0;
    float animDir = 1;

    // Getter
    public boolean GetFlying() 
    {
        return flying;
    }
    public float GetSpeed() 
    {
        return speed;
    }
    public float GetTargetSpeed() 
    {
        return targetSpeed;
    }
    
    // Constructor
    public WingController(DroneController controller, float speedIncrease, float speed, int updateFrequency) 
    {
        this.controller = controller;
        this.speedIncrease = speedIncrease;
        this.speed = speed;
        this.targetSpeed = speed;
        this.updateFrequency = updateFrequency;
    }

    // Toggle start / stop
    public boolean Toggle() 
    {
        if (flying) 
        {
            Stop();
        }
        else 
        {
            Start();
        }
        return flying;
    }

    // Start
    public void Stop()
    {
        controller.SendMotorInput(0,0);
        flying = false;
    }

    // Start 
    public void Start()
    {
        controller.SendMotorInput(0,speed);
        flying = true;
    }

    // Speed Up
    public float SpeedUp()
    {
        targetSpeed += 2 * speedIncrease;
        if (targetSpeed > 5.5) // MAX 6V 
        {
            targetSpeed = 5.5;
        }
        return targetSpeed;
    }

    // Speed Down
    public float SpeedDown()
    {
        targetSpeed -= 2 * speedIncrease;
        if (targetSpeed < 0.65)
        {
            targetSpeed = 0.65;
        }
        return targetSpeed;
    }

    // Step 
    public void Step() 
    {
        // Move servo to direction
        if (targetSpeed > speed) 
        {
            speed += speedIncrease;
            if (speed > targetSpeed) 
            {
                speed = targetSpeed;
            }
            if (frameCount % updateFrequency == 1 || speed == targetSpeed) 
            {
                controller.SendMotorInput(0, flying ? speed : 0);
            }
        }
        else if (targetSpeed < speed) 
        {
            speed -= speedIncrease;
            if (speed < targetSpeed) 
            {
                speed = targetSpeed;
            }
            if (frameCount % updateFrequency == 1 || speed == targetSpeed) 
            {
                controller.SendMotorInput(0, flying ? speed : 0);
            }
        }

        // Update speedition
        if (frameCount % (60 * updateFrequency) == 1) 
        {
            controller.SendMotorInput(0, flying ? speed : 0);
        }

        // Update animation
        if (flying) {
            anim += speed * speedIncrease * animDir * 20;
            if ((animDir == 1 && anim > 1) || (animDir == -1 && anim < 0)) 
            {
                animDir *= -1;
            }
        }

    }

    // Draw Wing Preview
    public void DrawPreview(int x, int y) 
    {
        float l = 160;

        push();
        translate(x, y + map(anim, 0, 1, -5, 2));

        // Body
        noStroke();
        fill(255,255,255,200);
        circle(0, 0, 60);

        // Right Wing
        push();
        noStroke();

        rotate(map(anim, 0, 1, 0, -PI/11));
        rect(-15, -15, l, 30, 30);

        push();
        translate(l-28, 0);
        rotate(map(anim, 0, 1, 0, PI/3));
        rect(-12, -12, l, 24, 24);
        pop();

        pop();

        // Left Wing
        push();
        noStroke();

        rotate(map(anim, 0, 1, PI, PI+PI/11));
        rect(-15, -15, l, 30, 30);

        push();
        translate(l-28, 0);
        rotate(map(anim, 0, 1, 0, -PI/3));
        rect(-12, -12, l, 24, 24);
        pop();

        pop();

        pop();
    }
}