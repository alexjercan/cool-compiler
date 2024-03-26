class Color {
    c: String;  -- the color is saved in 4 bytes

    init(r: Int, g: Int, b: Int, a: Int): SELF_TYPE {
        {
            r.chr().concat(g.chr()).concat(b.chr()).concat(a.chr());
            self;
        }
    };
};

class Raylib {
    initWindow(width: Int, height: Int, title: String): Object extern;
    windowShouldClose(): Bool extern;
    closeWindow(): Object extern;
    beginDrawing(): SELF_TYPE extern;
    endDrawing(): SELF_TYPE extern;
    clearBackground(color: Color): SELF_TYPE extern;
    drawText(text: String, x: Int, y: Int, fontSize: Int, color: Color): SELF_TYPE extern;
};
