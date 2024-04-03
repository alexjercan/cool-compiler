class Player {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;
    player_id: Int;

    player_id(): Int { player_id };

    init(x: Int, y: Int, sx: Int, sy: Int, pid: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            player_id <- pid;
            self;
        }
    };

    update(input: PlayerPosition): SELF_TYPE {
        {
            pos_x <- input.pos_x();
            pos_y <- input.pos_y();
            self;
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

    update(input: CoinPosition): SELF_TYPE {
        {
            pos_x <- input.pos_x();
            pos_y <- input.pos_y();
            self;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawCircle(pos_x, pos_y, radius, raylib.gold())
    };
};

class PlayerLobby inherits Thread {
    client: Client;
    player_id: Int;

    coin: Coin <- new Coin.init(0, 0, new Float.from_int(10));
    players: List (* Player *) <- new List.single(new Object);

    player_id(): Int { player_id };

    init(c: Client): SELF_TYPE { { client <- c; self; } };

    run(): Object {
        while true loop -- TODO: add exit condition
            case client.recv() of
                message: PlayerPosition => player_update(message);
                message: CoinPosition => coin_update(message);
                message: PlayerConnected => add_player(message);
                message: PlayerAuthorize => authorize_player(message);
                message: Message => 0;
            esac
        pool
    };

    player_update(msg: PlayerPosition): Object {
        {
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => if player.player_id() = msg.player_id() then player.update(msg) else 0 fi;
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
        }
    };

    coin_update(msg: CoinPosition): Object { coin.update(msg) };

    spawn_player(id: Int): Object { players <- players.append(new Player.init(0, 0, 50, 50, id)) };

    add_player(msg: PlayerConnected): Object {
        {
            new IO.out_string("Player connected ".concat(msg.player_id().to_string()).concat("\n"));
            spawn_player(msg.player_id());
        }
    };

    authorize_player(msg: PlayerAuthorize): Object {
        {
            new IO.out_string("Player authorize ".concat(msg.player_id().to_string()).concat("\n"));
            player_id <- msg.player_id();
            spawn_player(player_id);
        }
    };

    send(msg: Message): Object { {
        client.send(msg); } };

    draw(raylib: Raylib): Raylib {
        {
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => player.draw(raylib);
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
            coin.draw(raylib);
        }
    };
};

class Main {
    raylib: Raylib <- new Raylib;

    minX: Int <- 0;
    maxX: Int <- 16;
    minY: Int <- 0;
    maxY: Int <- 12;

    cell_size: Int <- 50;
    screen_width: Int <- maxX * cell_size;
    screen_height: Int <- maxY * cell_size;

    client: Client <- new Client.init("127.0.0.1", 8080, new MessageHelper).connect();
    lobby: PlayerLobby <- new PlayerLobby.init(client);

    pthread: PThread <- new PThread;
    lobby_thread: Int <- pthread.spawn(lobby);

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Coin Chase 2D").setTargetFPS(30);
            while raylib.windowShouldClose() = false loop
            {
                let keyA: Bool <- raylib.isKeyPressed(raylib.keyA()),
                    keyD: Bool <- raylib.isKeyPressed(raylib.keyD()),
                    keyW: Bool <- raylib.isKeyPressed(raylib.keyW()),
                    keyS: Bool <- raylib.isKeyPressed(raylib.keyS()),
                    message: PlayerInput <- new PlayerInput.init(lobby.player_id(), keyA, keyD, keyW, keyS)
                in lobby.send(message);

                raylib.beginDrawing();
                raylib.clearBackground(raylib.raywhite());

                -- TODO: add score UI

                lobby.draw(raylib);

                raylib.endDrawing();
            }
            pool;
            raylib.closeWindow();
            pthread.join(lobby_thread);
        }
    };
};
