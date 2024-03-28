class Player {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;

    init(x: Int, y: Int, sx: Int, sy: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            self;
        }
    };

    pos_x(): Int {
        pos_x
    };

    pos_y(): Int {
        pos_y
    };

    update(raylib: Raylib): Raylib {
        {
            if raylib.isKeyPressed(raylib.keyA()) then pos_x <- pos_x - size_x else
            if raylib.isKeyPressed(raylib.keyD()) then pos_x <- pos_x + size_x else
            if raylib.isKeyPressed(raylib.keyW()) then pos_y <- pos_y - size_y else
            if raylib.isKeyPressed(raylib.keyS()) then pos_y <- pos_y + size_y else 0 fi fi fi fi;
            raylib;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawRectangle(pos_x - size_x / 2, pos_y - size_y / 2, size_x, size_y, raylib.black())
    };
};

class Coin {
    pos_x: Int;
    pos_y: Int;
    radius: Float;

    init(x: Int, y: Int, r: Float): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            radius <- r;
            self;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawCircle(pos_x, pos_y, radius, raylib.gold())
    };

    update(raylib: Raylib, player: Player): Raylib {
        {
            if (pos_x = player.pos_x()).and(pos_y = player.pos_y()) then
            {
                pos_x <- raylib.getRandomValue(0, 5) * 100;
                pos_y <- raylib.getRandomValue(0, 5) * 100;
            }
            else 0 fi;
            raylib;
        }
    };
};

class Main {
    screen_width: Int <- 800;
    screen_height: Int <- 600;
    player_size: Int <- 50;

    raylib: Raylib <- new Raylib;
    player: Player <- new Player.init(screen_width / 2, screen_height / 2, player_size, player_size);
    coin: Coin <- new Coin.init(100, 100, new Float.from_int(10));

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Minecraft 2D").setTargetFPS(30).setRandomSeed(0);
            while raylib.windowShouldClose() = false loop
            {
                player.update(raylib);
                coin.update(raylib, player);

                raylib.beginDrawing();
                raylib.clearBackground(raylib.raywhite());

                player.draw(raylib);
                coin.draw(raylib);

                raylib.endDrawing();
            }
            pool;
            raylib.closeWindow();
        }
    };
};
