class SnakeCell {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;

    pos_x(): Int {
        pos_x
    };

    pos_y(): Int {
        pos_y
    };

    init(x: Int, y: Int, sx: Int, sy: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            self;
        }
    };

    set_position(x: Int, y: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            self;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawRectangle(pos_x - size_x / 2, pos_y - size_y / 2, size_x, size_y, raylib.black())
    };
};

class Snake {
    cells: List;
    cell_size: Int;

    snake_cell_size_x: Int;
    snake_cell_size_y: Int;

    min_x: Int <- 0;
    max_x: Int <- 5;
    min_y: Int <- 0;
    max_y: Int <- 5;

    speed_x: Int <- 1;
    speed_y: Int <- 0;

    frame_counter: Int <- 0;
    move_frame: Int <- 10;

    pressedA: Bool <- false;
    pressedD: Bool <- false;
    pressedW: Bool <- false;
    pressedS: Bool <- false;

    head_pos_x(): Int {
        (case cells.value() of cell: SnakeCell => cell; esac).pos_x()
    };

    head_pos_y(): Int {
        (case cells.value() of cell: SnakeCell => cell; esac).pos_y()
    };

    new_x(): Int {
        let x: Int <- head_pos_x() + speed_x * cell_size
        in
            if x < min_x * cell_size then max_x * cell_size
            else if max_x * cell_size < x then min_x * cell_size
            else x fi fi
    };

    new_y(): Int {
        let y: Int <- head_pos_y() + speed_y * cell_size
        in
            if y < min_y * cell_size then max_y * cell_size
            else if max_y * cell_size < y then min_y * cell_size
            else y fi fi
    };

    init(x: Int, y: Int, sx: Int, sy: Int, size: Int, xm: Int, xM: Int, ym: Int, yM: Int): SELF_TYPE {
        {
            let void: List in cells <- new List.init(new SnakeCell.init(x, y, sx, sy), void);
            cell_size <- size;
            snake_cell_size_x <- sx;
            snake_cell_size_y <- sy;
            min_x <- xm;
            max_x <- xM;
            min_y <- ym;
            max_y <- yM;
            self;
        }
    };

    dead(): Bool {
        let iter: List <- cells.next(),
            head: SnakeCell <- case cells.value() of cell: SnakeCell => cell; esac,
            x: Int <- head.pos_x(),
            y: Int <- head.pos_y(),
            dead: Bool <- false
        in
            {
                while not isvoid iter loop
                    {
                        let cell: SnakeCell <- case iter.value() of cell: SnakeCell => cell; esac
                        in
                            dead <- dead.or((x = cell.pos_x()).and(y = cell.pos_y()));

                        iter <- iter.next();
                    }
                pool;

                dead;
            }
    };

    grow(): SELF_TYPE {
        let iter: List <- cells,
            x: Int <- head_pos_x(),
            y: Int <- head_pos_y()
        in
            {
                while not isvoid iter.next() loop
                    {
                        let cell: SnakeCell <- case iter.value() of cell: SnakeCell => cell; esac
                        in
                            {
                                x <- cell.pos_x();
                                y <- cell.pos_y();
                            };

                        iter <- iter.next();
                    }
                pool;

                iter.append(new SnakeCell.init(x, y, snake_cell_size_x, snake_cell_size_y));

                self;
            }
    };

    move_snake(): Object {
        let iter: List <- cells,
            x: Int <- head_pos_x(),
            y: Int <- head_pos_y(),
            new_x: Int <- new_x(),
            new_y: Int <- new_y()
        in
            while not isvoid iter loop
                {
                    let cell: SnakeCell <- case iter.value() of cell: SnakeCell => cell; esac
                    in
                        {
                            x <- cell.pos_x();
                            y <- cell.pos_y();
                            cell.set_position(new_x, new_y);
                            new_x <- x;
                            new_y <- y;
                        };

                    iter <- iter.next();
                }
            pool
    };

    update(raylib: Raylib): Bool {
        {
            if raylib.isKeyPressed(raylib.keyA()) then pressedA <- true else
            if raylib.isKeyPressed(raylib.keyD()) then pressedD <- true else
            if raylib.isKeyPressed(raylib.keyW()) then pressedW <- true else
            if raylib.isKeyPressed(raylib.keyS()) then pressedS <- true else
            0 fi fi fi fi;

            frame_counter <- frame_counter + 1;
            if frame_counter = move_frame then
            {
                if pressedA.and(speed_y = 1) then { speed_x <- ~1; speed_y <- 0; } else
                if pressedA.and(speed_y = ~1) then { speed_x <- ~1; speed_y <- 0; } else
                if pressedD.and(speed_y = 1) then { speed_x <- 1; speed_y <- 0; } else
                if pressedD.and(speed_y = ~1) then { speed_x <- 1; speed_y <- 0; } else
                if pressedW.and(speed_x = 1) then { speed_x <- 0; speed_y <- ~1; } else
                if pressedW.and(speed_x = ~1) then { speed_x <- 0; speed_y <- ~1; } else
                if pressedS.and(speed_x = 1) then { speed_x <- 0; speed_y <- 1; } else
                if pressedS.and(speed_x = ~1) then { speed_x <- 0; speed_y <- 1; } else
                0 fi fi fi fi fi fi fi fi;

                pressedA <- false;
                pressedD <- false;
                pressedW <- false;
                pressedS <- false;

                move_snake();
                frame_counter <- 0;

                not dead();
            } else true fi;
        }
    };

    draw(raylib: Raylib): Raylib {
        {
            let iter: List <- cells
            in
                while not isvoid iter loop
                    {
                        case iter.value() of cell: SnakeCell => cell.draw(raylib); esac;
                        iter <- iter.next();
                    }
                pool;
            raylib;
        }
    };
};

class Coin {
    pos_x: Int;
    pos_y: Int;
    radius: Float;
    enabled: Bool <- false;

    cell_size: Int;

    min_x: Int <- 0;
    max_x: Int <- 5;
    min_y: Int <- 0;
    max_y: Int <- 5;

    pos_x(): Int { pos_x };

    pos_y(): Int { pos_y };

    consume(): SELF_TYPE { { enabled <- false; self; } };

    init(x: Int, y: Int, s: Float, size: Int, xm: Int, xM: Int, ym: Int, yM: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            radius <- s;
            cell_size <- size;
            min_x <- xm;
            max_x <- xM;
            min_y <- ym;
            max_y <- yM;
            self;
        }
    };

    update(raylib: Raylib): Object {
        {
            if not enabled then
            {
                pos_x <- raylib.getRandomValue(min_x, max_x) * cell_size;
                pos_y <- raylib.getRandomValue(min_y, max_y) * cell_size;
                enabled <- true;
            }
            else 0 fi;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawCircle(pos_x, pos_y, radius, raylib.gold())
    };
};

class Game {
    snake: Snake;
    coin: Coin;
    score: Int <- 0;

    init(s: Snake, c: Coin): SELF_TYPE {
        {
            snake <- s;
            coin <- c;
            self;
        }
    };

    update(raylib: Raylib): Bool {
        {
            if (snake.head_pos_y() = coin.pos_y()).and(snake.head_pos_x() = coin.pos_x()) then { coin.consume(); snake.grow(); score <- score + 1; }
            else 0 fi;

            coin.update(raylib);
            snake.update(raylib);
        }
    };

    draw(raylib: Raylib): Raylib {
        {
            raylib.beginDrawing();
            raylib.clearBackground(raylib.raywhite());

            snake.draw(raylib);
            coin.draw(raylib);

            raylib.drawText("Score: ".concat(score.to_string().concat("\n")), 10, 10, 20, raylib.black());

            raylib.endDrawing();

            raylib;
        }
    };
};

class Main {
    minX: Int <- 0;
    maxX: Int <- 16;
    minY: Int <- 0;
    maxY: Int <- 12;

    cell_size: Int <- 50;
    screen_width: Int <- maxX * cell_size;
    screen_height: Int <- maxY * cell_size;

    snake_cell_size: Int <- 45;
    coin_radius: Float <- new Float.from_int(10);

    raylib: Raylib <- new Raylib;
    snake: Snake <- new Snake.init(screen_width / 2, screen_height / 2, snake_cell_size, snake_cell_size, cell_size, minX + 1, maxX - 1, minY + 1, maxY - 1);
    coin: Coin <- new Coin.init(0, 0, coin_radius, cell_size, minX + 1, maxX - 1, minY + 1, maxY - 1);
    game: Game <- new Game.init(snake, coin);

    keep_running: Bool <- true;

    time: Time <- new Time;

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Snake 2D").setTargetFPS(30).setRandomSeed(time.time());
            while (not raylib.windowShouldClose()).and(keep_running) loop
            {
                keep_running <- game.update(raylib);
                game.draw(raylib);
            }
            pool;
            raylib.closeWindow();
        }
    };
};
