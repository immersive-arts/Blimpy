public class TailController 
{
    // Settings
    float startRange = 0;
    float endRange = 1;

    // Member Variables
    DroneController controller;
    float targetDir = 0;
    float pos = 0;
    float speed = 0.01;
    int updateFrequency = 1;

    // Getter
    public float GetPos() 
    {
        return pos;
    }
    
    // Constructor
    public TailController(DroneController controller, float speed, int updateFrequency) 
    {
        this.controller = controller;
        this.speed = speed;
        this.updateFrequency = updateFrequency;
    }

    // Move direction (right 1 left -1)
    public void MoveDirection(float dir) 
    {
        targetDir = dir;
    }

    // Move slowly right
    public float MoveRight() 
    {
        targetDir = pos + 2 * speed;
        if (targetDir > 1) 
        {
            targetDir = 1;
        }
        return targetDir;
    }

    // Move slowly left
    public float MoveLeft() 
    {
        targetDir = pos - 2 * speed;
        if (targetDir < -1) 
        {
            targetDir = -1;
        }
        return targetDir;
    }

    // Step 
    public void Step() 
    {
        // Move servo to direction
        if (targetDir > pos) 
        {
            pos += speed;
            if (pos > targetDir) 
            {
                pos = targetDir;
            }
            if (frameCount % updateFrequency == 1 || pos == targetDir) {
                controller.SendServoInput(0, map(pos, -1, 1, startRange, endRange));
                controller.SendServoInput(1, map(pos, 1, -1, startRange, endRange));
            }
        }
        else if (targetDir < pos) 
        {
            pos -= speed;
            if (pos < targetDir) 
            {
                pos = targetDir;
            }
            if (frameCount % updateFrequency == 1 || pos == targetDir) {
                controller.SendServoInput(0, map(pos, -1, 1, startRange, endRange));
                controller.SendServoInput(1, map(pos, 1, -1, startRange, endRange));
            }
        }

        // Update Position
        if (frameCount % (60 * updateFrequency) == 1) 
        {
            controller.SendServoInput(0, map(pos, -1, 1, startRange, endRange));
                controller.SendServoInput(1, map(pos, 1, -1, startRange, endRange));
        }
    }

    // Draw Tail Preview
    public void DrawPreview(int x, int y) 
    {
        // Middle Line
        stroke(0,0,0,50);
        strokeWeight(2);
        line(x, y - 150, x, y + 150);

        fill(255,255,255,200);

        // Tale Shape
        push();
        noStroke();
        translate(x, y - 100);
        fill(255,255,255,200);
        rotate(map(pos, -1, 1, PI/8, -PI/8));

        beginShape();
        vertex(0,0);
        vertex(120, 180);
        vertex(50, 195);
        vertex(0, 200);
        vertex(-50, 195);
        vertex(-120, 180);
        endShape(CLOSE);
        
        rect(-15, -15, 30, 200, 30);

        pop();
    }
}