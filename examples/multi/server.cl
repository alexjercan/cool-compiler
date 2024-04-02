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

    update(input: PlayerInput): Object {
        if (input.keyA()) then pos_x <- pos_x - size_x else
        if (input.keyD()) then pos_x <- pos_x + size_x else
        if (input.keyW()) then pos_y <- pos_y - size_y else
        if (input.keyS()) then pos_y <- pos_y + size_y else 0 fi fi fi fi
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

    update(): Object {
        {
            if not enabled then
            {
                pos_x <- (new Random.random().mod(max_x - min_x) + min_x) * cell_size;
                pos_y <- (new Random.random().mod(max_y - min_y) + min_y) * cell_size;
                enabled <- true;
            }
            else 0 fi;
        }
    };
};

class PlayerLobby inherits Thread {
    server: Server;

    coin: Coin;
    players: List <- new List.single(new Object);

    players(): List { players };

    init(s: Server): SELF_TYPE { { server <- s; self; } };

    run(): Object {
        while true loop
            let clientfd: Int <- server.accept()
            in
                {
                    new IO.out_string("Client ".concat(clientfd.to_string()).concat(" connected\n"));

                    broadcast(new PlayerConnected.init(clientfd));
                    send(clientfd, new PlayerAuthorize.init(clientfd));
                    let iter: List <- players
                    in
                        while not isvoid iter loop
                            {
                                case iter.value() of
                                    player: Int => send(clientfd, new PlayerConnected.init(player));
                                    player: Object => 0;
                                esac;
                                iter <- iter.next();
                            }
                        pool;

                    players <- players.append(clientfd);
                    -- TODO: Maybe spawn thread for player?
                    -- TODO: Implement game loop (receive messages, update game state, send messages)
                }
        pool
    };

    send(player: Int, msg: Message): Object { server.send(player, msg) };

    broadcast(msg: Message): Object {
        let iter: List <- players
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        player: Int => send(player, msg);
                        player: Object => 0;
                    esac;
                    iter <- iter.next();
                }
            pool
    };
};

class Main {
    linux: Linux <- new Linux;
    server: Server <- new Server.init(8080).listen(10);
    pthread: PThread <- new PThread;
    lobby: PlayerLobby <- new PlayerLobby.init(server);
    lobby_thread: Int <- pthread.spawn(lobby);

    main(): Object {
        {
            new IO.out_string("Listening on port 8080\n");

            pthread.join(lobby_thread);
        }
    };
};
