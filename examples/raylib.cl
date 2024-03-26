class Main {
    raylib: Raylib <- new Raylib;
    raywhite: Color <- new Color.init(255, 0, 0, 255);

    main(): Object {
        {
            raylib.initWindow(800, 600, "Raylib from COOL");
            while not raylib.windowShouldClose() loop
                raylib
                    .beginDrawing()
                    .clearBackground(raywhite)
                    .drawText("Congrats! You created your first window!", 190, 200, 20, raywhite)
                    .endDrawing()
            pool;
            raylib.closeWindow();
        }
    };
};
