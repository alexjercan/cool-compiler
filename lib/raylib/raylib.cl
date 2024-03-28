class Color {
    c: Int; -- the color in RGBA format

    init(r: Int, g: Int, b: Int, a: Int): SELF_TYPE {
        {
            c <- a * 256 * 256 * 256 + b * 256 * 256 + g * 256 + r;
            self;
        }
    };
};

class Raylib {
    initWindow(width: Int, height: Int, title: String): SELF_TYPE extern;
    windowShouldClose(): Bool extern;
    closeWindow(): SELF_TYPE extern;
    beginDrawing(): SELF_TYPE extern;
    endDrawing(): SELF_TYPE extern;
    clearBackground(color: Color): SELF_TYPE extern;
    drawText(text: String, x: Int, y: Int, fontSize: Int, color: Color): SELF_TYPE extern;
    setTargetFPS(fps: Int): SELF_TYPE extern;

    -- Basic shapes drawing functions
    drawCircle(centerX: Int, centerY: Int, radius: Float, color: Color): SELF_TYPE extern;
    drawRectangle(x: Int, y: Int, width: Int, height: Int, color: Color): SELF_TYPE extern;

    -- Input-related functions: keyboard
    isKeyPressed(key: Int): Bool extern; -- Check if a key has been pressed once

    -- Keyboard keys (US keyboard layout)
    keyA(): Int { 65 };
    keyD(): Int { 68 };
    keyS(): Int { 83 };
    keyW(): Int { 87 };

    -- Some Basic Colors
    gold(): Color { new Color.init(255, 203, 0, 255) };
    lightgray(): Color { new Color.init(200, 200, 200, 255) };

    black(): Color { new Color.init(0, 0, 0, 255) };
    raywhite(): Color { new Color.init(245, 245, 245, 255) };

    -- Random values generation functions
    setRandomSeed(seed: Int): SELF_TYPE extern;
    getRandomValue(min: Int, max: Int): Int extern;
};
