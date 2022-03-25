/**
    Button class to easily handle Button events
    @author Ivo Keller aka. Gonios
 */
public class Button {
    // Variables
    private int x;
    private int y;
    private int w;
    private int h;
    private color defaultColor;
    private color hoverColor;
    private boolean centered = false;

    // Properties
    private String text;
    private boolean selected = false;

    // Get
    public String GetText() {
        return text;
    }
    public boolean GetSelected() {
        return selected;
    }
    
    // Set
    public void SetText(String text) {
        this.text = text;
    }
    public void SetSelected(boolean selected) {
        this.selected = selected;
    }

    // Constructor
    public Button(String text, int x, int y, int w, int h, color defaultColor, color hoverColor, boolean centered) {
        this.text = text;
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
        this.defaultColor = defaultColor;
        this.hoverColor = hoverColor;
        this.centered = centered;
    }
    public Button(String text, int x, int y, int w, int h, color defaultColor, color hoverColor) {
        this(text, x, y, w, h, defaultColor, hoverColor, false);
    }
    public Button(String text, int x, int y, int w, int h, boolean centered) {
        this(text, x, y, w, h, color(255,255,255,200), color(255,255,254), centered);
    }
    public Button(String text, int x, int y, int w, int h) {
        this(text, x, y, w, h, false);
    }

    // Draw
    public void draw() {
        // Calc Centered X Pos
        int xPos = centered ? x-w/2 : x;
        // Hover
        if(isMouseOver()) {
            fill(hoverColor);
        }
        else {
            fill(!selected ? defaultColor : hoverColor);
        }
        // Draw Button
        noStroke();
        rect(xPos, y, w, h, h/15);
        // Text
        fill(60);
        textAlign(CENTER, CENTER);
        textSize(height/25);
        text(text, xPos+10, y+5, w-20, h-20);
    }

    // Check Mouse over
    public boolean isMouseOver() {
        int xPos = centered ? x-w/2 : x;
        int xWPos = centered ? x+w/2 : x+w;
        return mouseX >= xPos && mouseX <= xWPos && mouseY >= y && mouseY <= y+h;
    }
}