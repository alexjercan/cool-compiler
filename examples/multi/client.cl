class Player {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;
    player_id: Int;

    score: Int <- 0;

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

    update_score(input: PlayerScore): SELF_TYPE { { score <- input.score(); self; } };

    draw(raylib: Raylib, color: Color): Raylib {
        {
            raylib.drawRectangle(pos_x - size_x / 2, pos_y - size_y / 2, size_x, size_y, color);
            let text: String <- score.to_string(),
                font_size: Int <- 20,
                text_size: Int <- raylib.measureText(text, font_size),
                text_x: Int <- pos_x - text_size / 2,
                text_y: Int <- pos_y - font_size / 2
            in raylib.drawText(text, text_x, text_y, font_size, raylib.black());
        }
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
    raylib: Raylib;

    player_id: Int;
    keep_running: Bool <- true;

    fight_lose: Bool <- false;
    fight_win: Bool <- false;

    coin: Coin <- new Coin.init(0, 0, new Float.from_int(10));
    players: List (* Player *) <- new List.single(new Object);

    player_id(): Int { player_id };

    init(c: Client, r: Raylib): SELF_TYPE { { client <- c; raylib <- r; self; } };

    run(): Object {
        while (not raylib.windowShouldClose()).and(keep_running) loop
            case client.recv() of
                message: PlayerPosition => player_update(message);
                message: CoinPosition => coin_update(message);
                message: PlayerConnected => add_player(message);
                message: PlayerAuthorize => authorize_player(message);
                message: PlayerScore => score_update(message);
                message: PlayerDisconnected => remove_player(message);
                message: PlayerFightLose => fight_lose <- true;
                message: PlayerFightWin => fight_win <- true;
                message: DisconnectedMessage => { keep_running <- false; client.close(); };
                message: Message => abort();
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

    score_update(msg: PlayerScore): Object {
        {
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => if player.player_id() = msg.player_id() then player.update_score(msg) else 0 fi;
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
        }
    };

    remove_player(msg: PlayerDisconnected): Object {
        let iter: List (* Player *) <- players.next(),
            prev: List (* Player *) <- players
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        p: Player => {
                            if p.player_id() = msg.player_id() then
                                {
                                    prev.set_next(iter.next());
                                    new IO.out_string("Client ".concat(p.player_id().to_string()).concat(" disconnected\n"));
                                }
                            else 0 fi;
                        };
                        p: Object => 0;
                    esac;
                    iter <- iter.next();
                    prev <- prev.next();
                }
            pool
    };

    send(msg: Message): Object { { client.send(msg); } };

    draw(raylib: Raylib): Raylib {
        {
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => if player.player_id() = player_id then player.draw(raylib, raylib.green()) else player.draw(raylib, raylib.red()) fi;
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
            coin.draw(raylib);

            if fight_lose then { raylib.clearBackground(raylib.red()); fight_lose <- false; } else 0 fi;
            if fight_win then { new IO.out_string("*really nice sounds and particles*\n"); fight_win <- false; } else 0 fi;

            raylib;
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
    lobby: PlayerLobby <- new PlayerLobby.init(client, raylib);

    pthread: PThread <- new PThread;
    lobby_thread: Int;

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Coin Chase 2D").setTargetFPS(30);
            lobby_thread <- pthread.spawn(lobby);
            while (not raylib.windowShouldClose()).and(not client.is_closed()) loop
            {
                let keyA: Bool <- raylib.isKeyPressed(raylib.keyA()),
                    keyD: Bool <- raylib.isKeyPressed(raylib.keyD()),
                    keyW: Bool <- raylib.isKeyPressed(raylib.keyW()),
                    keyS: Bool <- raylib.isKeyPressed(raylib.keyS()),
                    message: PlayerInput <- new PlayerInput.init(lobby.player_id(), keyA, keyD, keyW, keyS)
                in lobby.send(message);

                raylib.beginDrawing();
                raylib.clearBackground(raylib.raywhite());

                lobby.draw(raylib);

                raylib.endDrawing();
            }
            pool;
            raylib.closeWindow();
            pthread.join(lobby_thread);

            new IO.out_string("Closing client...\n");
        }
    };
};
