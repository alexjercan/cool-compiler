class Main {
    raylib: Raylib <- new Raylib;

    main(): Object {
        {
            raylib.initWindow(800, 600, "Raylib from COOL").setTargetFPS(30);
            while raylib.windowShouldClose() = false loop
            {
                raylib
                    .beginDrawing()
                    .clearBackground(raylib.raywhite())
                    .drawText("Congrats! You created your first window!", 190, 200, 20, raylib.black())
                    .drawRectangle(10, 10, 10, 10, raylib.black())
                    .endDrawing();
            }
            pool;
            raylib.closeWindow();
        }
    };
};
